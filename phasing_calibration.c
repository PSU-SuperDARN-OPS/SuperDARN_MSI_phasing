
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "include/phasing_cards.h"
#include "include/utils.h"
#include "include/pci_dio_120.h"
#include "include/vna_functions.h"


#define CARDS 200
#define PHASECODES 8192
#define ATTENCODES 64

int verbose = 2;
char *hostip = "192.168.1.2";
char *file_prefix = "phasing_cal";
char *file_ext = ".dat";
char filename[120] = "";
char *dir = "/data/cal/";
FILE *calfile = NULL;
int port = 23;
char radar_name[80] = "";
char freq_start[10] = "8E6";
char freq_stop[10] = "20E6";
char freq_steps[10] = "201";


int main(int argc, char **argv) {
    double *phase[VNA_FREQS], *pwr_mag[VNA_FREQS];
    double freq[VNA_FREQS];
    double pd_old, pd_new, phase_diff = 0.0;
    int32_t count;
    int32_t i = 0, c = 31, data = 0, wait_delay_ms = 30;
    int b = 0;
    int last_collect, current_collect, collect = 0, beamcode = 0, take_data = 0, attempt = 0, max_attempts = 20;
    double fstart;
    double fstop;
    double fstep;
    int fnum;
    uint_fast8_t radar;
    char serial_number[80] = "";
    int temp;

    struct DIO phasing_matrix;

    if (argc < 2) {
        fprintf(stderr, "%s: invoke with radar number (1 or 2 or 3)\n", argv[0]);
        fflush(stderr);
        exit(0);
    }

    radar = atoi(argv[1]);
    printf("Radar: %d Card: %d\n", radar, c);

    phasing_matrix.radar_number = radar;
    temp = set_ports(&phasing_matrix);

    if (temp == -1) {
        printf("Error: Incorrect radar number");
        exit(-1);
    }

    init_pci_dio_120();

    /* INITIALIZE THE CARD FOR PROPER IO */
    init_phasing_cards(&phasing_matrix);
    select_card(&phasing_matrix, c);

    fnum = atoi(freq_steps);
    fstart = atof(freq_start);
    fstop = atof(freq_stop);
    fstep = (fstop - fstart) / (fnum - 1);

    for (i = 0; i < fnum; i++) {
        freq[i] = fstart + i * fstep;
        phase[i] = calloc(PHASECODES, sizeof(double));
        pwr_mag[i] = calloc(PHASECODES, sizeof(double));
    }

    // Open Socket and initial IO
    if (verbose > 0) printf("Opening Socket %s %d\n", hostip, port);
    init_vna(hostip, port);

    calibrate_vna(freq_start, freq_stop, freq_steps);

    vna_trigger_single(verbose);
    printf("\n\nCalibration Complete\nReconfigure for Phasing Card Measurements\n");
    mypause();

    c = -1;
    printf("\n\nEnter Radar Name: ");
    fflush(stdin);
    scanf("%s", radar_name);
    fflush(stdout);
    fflush(stdin);
    printf("\n\nEnter Phasing Card Number: ");
    fflush(stdin);
    fflush(stdout);
    scanf("%d", &c);
    printf("\n\nEnter Serial Number: ");
    fflush(stdin);
    fflush(stdout);
    scanf("%s", serial_number);
    printf("Radar: <%s>  Card: %d Serial: %s\n", radar_name, c, serial_number);
    fflush(stdout);



    sprintf(filename, "%s%s_%s_%02d_%s%s", dir, file_prefix, radar_name, c, serial_number, file_ext);
    if (verbose > 0) fprintf(stdout, "Using file: %s\n", filename);
    fflush(stdout);

    calfile = fopen(filename, "w");
    if (calfile == NULL) {
        perror("Error in opening calibration file, "
               "make sure /data/cal is created and permissions set to allow writing");
        exit(-1);
    }

    if (verbose > 0) {
        fprintf(stdout, "Writing beamcodes to phasing card\n");
    }
    count = PHASECODES;
    fwrite(&count, sizeof(int32_t), 1, calfile);
    count = CARDS;
    fwrite(&count, sizeof(int32_t), 1, calfile);
    count = fnum;
    fwrite(&count, sizeof(int32_t), 1, calfile);
    count = 0;
    fwrite(freq, sizeof(double), fnum, calfile);
        usleep(10000);



    printf("Programming all zeros attenuation coding\n");
    for (b = 0; b < ATTENCODES; b++) {
        data = 0;
        beamcode = b;

        temp = write_data(&phasing_matrix, c, beamcode, data);
        temp = write_attenuators(&phasing_matrix, c, beamcode, data);
    }

    printf("Verifying all zero programming attenuation coding\n");
    for (b = 0; b < ATTENCODES; b++) {
        temp = verify_attenuators(&phasing_matrix, c, b, 0);
    }

    printf("Programming 1-to-1 attenuation coding no phase\n");
    for (b = 0; b < ATTENCODES; b++) {
        data = b;
        beamcode = b;

        temp = write_data(&phasing_matrix, c, beamcode, 0);
        temp = write_attenuators(&phasing_matrix, c, beamcode, b);
    }

    printf("Verifying 1-to-1 programming attenuation coding\n");
    for (b = 0; b < ATTENCODES; b++) {
        temp = verify_attenuators(&phasing_matrix, c, b, b);
    }

    printf("Programming all ones phase coding\n");
    for (b = 0; b < PHASECODES; b++) {
        data = 8191;
        beamcode = b;

        if (b % 512 == 0) printf("B: %d data: %d BC: %d\n", b, data, beamcode);
        temp = write_data(&phasing_matrix, c, beamcode, 8191);
        temp = write_attenuators(&phasing_matrix, c, beamcode, 63);
    }
    printf("Verifying all ones programming phase coding\n");
    for (b = 0; b < PHASECODES; b++) {
        temp = verify_data(&phasing_matrix, c, b, 8191);
    }

    printf("Programming 1-to-1 phase coding no attenuation\n");
    for (b = 0; b < PHASECODES; b++) {
        data = b;
        beamcode = b;

        if (b % 512 == 0) printf("B: %d data: %d BC: %d\n", b, data, beamcode);
        temp = write_data(&phasing_matrix, c, beamcode, 0);
        temp = write_data(&phasing_matrix, c, beamcode, data);
        temp = write_attenuators(&phasing_matrix, c, beamcode, 0);
    }
    printf("Verifying 1-to-1 programming phase coding\n");
    for (b = 0; b < PHASECODES; b++) {
        temp = verify_data(&phasing_matrix, c, b, b);
    }


    sleep(10);
    if (verbose > 0) fprintf(stdout, "Measuring phases\n");

    last_collect = 0;
    current_collect = 0;
    for (b = 0; b < PHASECODES; b++) {
        beamcode = b;
        temp = select_card(&phasing_matrix, c);
        select_beam_code(&phasing_matrix, beamcode);
        collect = 0;
        if (b == 0) collect = 1;
        if (b == (PHASECODES - 1)) collect = 1;
        if (b == 1) collect = 1;
        if (b == 2) collect = 1;
        if (b == 4) collect = 1;
        if ((b % 8) == 0) collect = 1;
        if (!collect) {
            printf(":::Card %d::: Skipping BEAMCode %d :: %d\n", c, beamcode, b % 8);
            continue;
        } else {
            printf(":::Card %d::: Reading BEAMCode %d :: %d\n", c, beamcode, b % 8);
            last_collect = current_collect;
            current_collect = b;
        }

        temp = verify_data(&phasing_matrix, c, b, b);

        if (temp != 0) {
            data = b;

            if (b % 512 == 0) printf("B: %d data: %d BC: %d\n", b, data, beamcode);
            temp = write_data(&phasing_matrix, c, beamcode, 0);
            temp = write_data(&phasing_matrix, c, beamcode, data);
            temp = write_attenuators(&phasing_matrix, c, beamcode, 0);

            usleep(10000);
            temp = verify_data(&phasing_matrix, c, b, b);
        }
        if (temp != 0) {
            printf("Failed Verification for beamcode: %d  %x\n", b, b);
            exit(-1);
        }

        vna_trigger_single(verbose);
        if (b == 0) sleep(1);

        usleep(wait_delay_ms * 1000);

        take_data = 1;
        attempt = 0;
        while ((take_data) && (attempt < max_attempts)) {

            attempt++;
            vna_get_data(1, phase, b, verbose);
            vna_get_data(2, pwr_mag, b, verbose);
            pd_new = phase[fnum - 1][b] - phase[0][b];
            if (b != 0) {
                pd_old = phase[fnum - 1][last_collect] - phase[0][last_collect];
                phase_diff = fabs(pd_new - pd_old);
                if (phase_diff > 1.0E-6) {
                    take_data = 0;
                } else {  //phase_diff too small
                    wait_delay_ms = wait_delay_ms + 10;
                    printf("Data Collection Error: %d. Increasing delay to %d (ms)\n", b, wait_delay_ms);
                    printf("  Phase Diff: %lf : %lf %lf\n", phase_diff, pd_old, pd_new);
                    usleep(wait_delay_ms);
                }
            } else {  //b==0
                take_data = 0;
            }
        }
        if (attempt == max_attempts) {
            printf("FATAL ERROR: Phasecode %d took Max attempts %d last delay %d (ms)\n", b, attempt,
                   wait_delay_ms);
            exit(0);
        }
        printf("Phasecode %d: Freq Index: %d Phase: %lf Pd: %lf Wait Delay: %d\n", b, fnum - 1, phase[fnum - 1][b],
               pd_new, wait_delay_ms);
    } // end of phasecode loop

    for (i = 0; i < fnum; i++) {
        if (verbose > 1)
            printf("Freq %lf:  Phase 0:%lf Phase 8191: %lf\n", freq[i], phase[i][0], phase[i][PHASECODES - 1]);
        if (verbose > 1)
            printf("Freq %lf:  Pwr 0:%lf Pwr 8191: %lf\n", freq[i], pwr_mag[i][0], pwr_mag[i][PHASECODES - 1]);
        fwrite(&i, sizeof(int32_t), 1, calfile);
        count = fwrite(phase[i], sizeof(double), PHASECODES, calfile);
        count = fwrite(pwr_mag[i], sizeof(double), PHASECODES, calfile);
        //printf("Freq index: %d Count: %d\n",i,count);
    }
    printf("Closing File\n");
    fclose(calfile);
    c = -1;
}




