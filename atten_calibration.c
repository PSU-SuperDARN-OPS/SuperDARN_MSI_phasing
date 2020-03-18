
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <ctype.h>


#include "include/registers.h"
#include "include/phasing_cards.h"
#include "include/utils.h"
#include "include/pci_dio_120.h"
#include "include/vna_functions.h"


#define CARDS 200
#define ATTENCODES 64
#define FREQS 1500


int main(int argc, char **argv) {
    double *atten_phase[FREQS], *atten_pwr_mag[FREQS];
    double freq[FREQS];
    double pd_old, pd_new, pwr_diff = 0.0;
    int32_t count;
    int32_t i = 0, card = 31, data = 0, wait_delay = 10;
    unsigned int b = 0;
    int last_collect, current_collect, beamcode = 0, take_data = 0, attempt = 0, max_attempts = 20;
    double fstart;
    double fstop;
    double fstep;
    int fnum;
    int radar_number;
    char serial_number[80];
    int temp;
    int verbose = 2;
    char *hostip = "192.168.1.2";
    char *file_prefix = "phasing_cal";
    char *file_ext = ".dat";
    char *atten_ext = ".att";
    char attenfilename[120];
    char *dir = "/data/cal/";
    FILE *attenfile = NULL;
    int port = 23;
    char radar_name[80];
    char freq_start[10] = "8E6";
    char freq_stop[10] = "20E6";
    char freq_steps[10] = "201";

    int rval, nflag = 0, iflag = 0, cflag = 0, rflag = 0, sflag = 0;

    struct DIO phasing_matrix;
/***************************************************************
     * COMMAND LINE ARGUMENTS
     **************************************************************/
    while ((rval = getopt(argc, argv, "+r:n:c:p:v:s:ih")) != -1) {
        switch (rval) {
            case 'v':
                verbose = atoi(optarg);
                break;
            case 'n':
                radar_number = atoi(optarg);
                nflag = 1;
                break;
            case 'c':
                card = atoi(optarg);
                cflag = 1;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'r':
                snprintf(radar_name, 16, "%s", optarg);
                rflag = 1;
                break;
            case 'i':
                iflag = 1;
                break;
            case 's':
                snprintf(serial_number, 16, "%s", optarg);
                sflag = 1;
                break;
            case '?':
                if (optopt == 'r' || optopt == 'c' || optopt == 'n' || optopt == 'p' || optopt == 's' ||
                    optopt == 'a' || optopt == 'v')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return 1;
            case 'h':
            default:
                fprintf(stderr,
                        "Required:\n  -r radarname\n  -n dio radar number (1 or 2)\n  -c card number\n  -s serial number\nOptional:\n  -a vna ipaddress\n  -p vna tcp port\n  -i to run vna init and cal process\n  -v number to set verbose output level\n");
                return 1;
        }
    }
    if (argc == 1 || radar_number == 0 || rflag == 0 || nflag == 0 || cflag == 0 || sflag == 0) {
        fprintf(stderr,
                "Required arguments -r radarname, -n dio radar number, -s serial number, and -c card number\n Consult the help using the -h option\n");
        return 1;
    }
    printf("Radar: %d Card: %d\n", radar_number, card);

    phasing_matrix.radar_number = radar_number;
    set_ports(&phasing_matrix);

    init_pci_dio_120();

    init_phasing_cards(&phasing_matrix);


    fnum = atoi(freq_steps);
    fstart = atof(freq_start);
    fstop = atof(freq_stop);
    fstep = (fstop - fstart) / (fnum - 1);
    for (i = 0; i < fnum; i++) {
        freq[i] = fstart + i * fstep;
        atten_phase[i] = calloc(ATTENCODES, sizeof(double));
        atten_pwr_mag[i] = calloc(ATTENCODES, sizeof(double));
    }

    // Open Socket and initial IO
    init_vna(hostip, port);
    vna_trigger_single(verbose);

    if(iflag) {
        calibrate_vna(freq_start, freq_stop, freq_steps);

        vna_trigger_single(verbose);
        printf("\n\nCalibration Complete\nReconfigure for Phasing Card Measurements");
        mypause();
    }


    printf("Radar: <%s>  Card: %d Serial: %s\n", radar_name, card, serial_number);
    fflush(stdout);


    sprintf(attenfilename, "%s%s_%s_%02d_%s%s", dir, file_prefix, radar_name, card, serial_number, atten_ext);
    if (verbose > 0) fprintf(stdout, "Using file: %s\n", attenfilename);
    fflush(stdout);
    attenfile = fopen(attenfilename, "w");
    count = ATTENCODES;
    fwrite(&count, sizeof(int32), 1, attenfile);
    count = CARDS;
    fwrite(&count, sizeof(int32), 1, attenfile);
    count = fnum;
    fwrite(&count, sizeof(int32), 1, attenfile);
    count = 0;
    fwrite(freq, sizeof(double), fnum, attenfile);
    if (verbose > 0) {
        fprintf(stdout, "Writing beamcodes to phasing card\n");
    }
    usleep(10000);


    printf("Programming all zeros attenuation coding\n");
    for (b = 0; b < ATTENCODES; b++) {
        data = 0;
        beamcode = b;

        temp = write_data(&phasing_matrix, card, beamcode, data);
        temp = write_attenuators(&phasing_matrix, card, beamcode, data);

    }

    printf("Verifying all zero programming attenuation coding\n");
    for (b = 0; b < ATTENCODES; b++) {
        select_card(&phasing_matrix, card);
        select_beam_code(&phasing_matrix, b);
        usleep(10000);
        temp = verify_attenuators(&phasing_matrix, card, b, 0);
    }

    printf("Programming 1-to-1 attenuation coding no phase\n");
    for (b = 0; b < ATTENCODES; b++) {
        data = b;
        beamcode = b;

        temp = write_data(&phasing_matrix, card, beamcode, 0);
        temp = write_attenuators(&phasing_matrix, card, beamcode, b);
    }
    printf("Verifying 1-to-1 programming attenuation coding\n");
    for (b = 0; b < ATTENCODES; b++) {
        select_card(&phasing_matrix, card);
        select_beam_code(&phasing_matrix, b);
        usleep(10000);
        temp = verify_attenuators(&phasing_matrix, card, b, b);
    }


    sleep(10);


    if (verbose > 0) fprintf(stdout, "Measuring attens\n");
    last_collect = 0;
    current_collect = 0;
    for (b = 0; b < ATTENCODES; b++) {
        beamcode = b;
        temp = select_card(&phasing_matrix, card);
        select_beam_code(&phasing_matrix, beamcode);
        vna_trigger_single(verbose);
        if (b == 0) sleep(1);

        usleep(wait_delay);

        take_data = 1;
        attempt = 0;
        while ((take_data) && (attempt < max_attempts)) {

            attempt++;
            vna_get_data(1, atten_phase, b, verbose);
            vna_get_data(2, atten_pwr_mag, b, verbose);
            pd_new = atten_pwr_mag[fnum - 1][b] - atten_pwr_mag[0][b];
            if (b != 0) {
                pd_old = atten_pwr_mag[fnum - 1][last_collect] - atten_pwr_mag[0][last_collect];
                pwr_diff = fabs(pd_new - pd_old);
                if (pwr_diff > 1.0E-6) {
                    take_data = 0;
                } else {  //pwr_diff too small
                    wait_delay = wait_delay + 10;
                    printf("Data Collection Error: %d. Increasing delay to %d (ms)\n", b, wait_delay);
                    printf("  Data Diff: %lf : %lf %lf\n", pwr_diff, pd_old, pd_new);
                    usleep(wait_delay);
                }
            } else {  //b==0
                take_data = 0;
            }
        }
        if (attempt == max_attempts) {
            printf("FATAL ERROR: Attencode %d took Max attempts %d last delay %d (ms)\n", b, attempt, wait_delay);
            exit(0);
        }
        printf("Attencode %d: Freq Index: %d Pwr: %lf Pd: %lf Wait Delay: %d\n", b, fnum - 1,
               atten_pwr_mag[fnum - 1][b], pd_new, wait_delay);
    } // end of attencode loop

    for (i = 0; i < fnum; i++) {
        if (verbose > 1)
            printf("Freq %lf:  Pwr 0:%lf Pwr %d: %lf\n", freq[i], atten_pwr_mag[i][0], (int) ATTENCODES - 1,
                   atten_pwr_mag[i][ATTENCODES - 1]);
        fwrite(&i, sizeof(int32), 1, attenfile);
        count = fwrite(atten_phase[i], sizeof(double), ATTENCODES, attenfile);
        count = fwrite(atten_pwr_mag[i], sizeof(double), ATTENCODES, attenfile);
    }

    for (i = 0; i < fnum; i++) {
        if (verbose > 1)
            printf("Freq %lf:  Pwr 0:%lf Pwr Max: %lf\n", freq[i], atten_pwr_mag[i][0],
                   atten_pwr_mag[i][ATTENCODES - 1]);
        fwrite(&i, sizeof(int32), 1, attenfile);
        count = fwrite(atten_phase[i], sizeof(double), ATTENCODES, attenfile);
        count = fwrite(atten_pwr_mag[i], sizeof(double), ATTENCODES, attenfile);

    }
    printf("Closing File\n");
    fclose(attenfile);
}

