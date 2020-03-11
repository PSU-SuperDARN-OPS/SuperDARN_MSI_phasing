#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>


#include "include/registers.h"
#include "include/phasing_cards.h"
#include "include/utils.h"
#include "include/pci_dio_120.h"
#include "include/vna_functions.h"

#define SWITCHES 0
#define ATTEN    1
#define READ     0
#define WRITE    1
#define ON       1
#define OFF      0


#define CARDS 200
#define ATTENCODES 64
#define FREQS 1500
#define _QUICK_

int stupid_flag = 0;
int setup_flag = 1;
int test_flag = -1000;
int sock = -1;
int verbose = 2;
char *hostip = "192.168.1.2";
char *file_prefix = "phasing_cal";
char *file_ext = ".dat";
char *atten_ext = ".att";
char attenfilename[120];
char *dir = "/data/cal/";
FILE *attenfile = NULL;
int port = 23;
char command[80];
char radar_name[80];
char freq_start[10] = "8E6";
char freq_stop[10] = "20E6";
char freq_steps[10] = "201";

struct timeval t0, t1, t2, t3, t4, t5, t6;
struct timeval t10, t11;
unsigned long elapsed;

int main(int argc, char **argv) {
    char output[40], strout[40];
    char cmd_str[80], prompt_str[10], data_str[1000];
    double *atten_phase[FREQS], *atten_pwr_mag[FREQS];
    double freq[FREQS];
    double pd_old, pd_new, pwr_diff = 0.0;
    int32 rval, count, sample_count, fail, cr, lf;
    int32 ii, i = 0, c = 31, data = 0, index = 0, wait_delay = 10;
    unsigned int b = 0;
    int last_collect, current_collect, collect = 0, beamcode = 0, take_data = 0, attempt = 0, max_attempts = 20;
    double fstart;
    double fstop;
    double fstep;
    int fnum;
    int radar;
    char serial_number[80];
    unsigned int portA0, portB0, portC0, cntrl0;
    unsigned int portA1, portB1, portC1, cntrl1;
    int temp, pci_handle, j, IRQ;
    unsigned char *BASE0, *BASE1;
    unsigned int mmap_io_ptr, CLOCK_RES;
    float time;

    struct DIO phasing_matrix;


    struct		 timespec start_p, stop_p, start, stop, nsleep;

    if(argc <2 ) {
      fprintf(stderr,"%s: invoke with radar number (1 or 2 or 3)\n",argv[0]);
      fflush(stderr);
      exit(0);
    }
    if(argc ==3 ) {
      if(atoi(argv[2])==0) setup_flag=0;
      else setup_flag=1;
    } else {
      test_flag=-1000;
    }
    if(argc ==4 ) {
      test_flag=atoi(argv[3]);
      c=atoi(argv[2]);
    } else {
      test_flag=-1000;
    }
    radar=atoi(argv[1]);
    printf("Radar: %d Card: %d\n",radar,c);
    printf("Test flag: %d\n",test_flag);

    phasing_matrix.radar_number = radar;
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

    calibrate_vna(freq_start, freq_stop, freq_steps);
    
    vna_button_command(":INIT1:IMM\r\n", 0, verbose);
    printf("\n\nCalibration Complete\nReconfigure for Phasing Card Measurements");
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
    
    while ((c < CARDS) && (c >= 0)) {
        
        sprintf(attenfilename, "%s%s_%s_%02d_%s%s", dir, file_prefix, radar_name, c, serial_number, atten_ext);
        if (verbose > 0) fprintf(stdout, "Using file: %s\n", attenfilename);
        fflush(stdout);
        gettimeofday(&t0, NULL);
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
            gettimeofday(&t2, NULL);
        }
        usleep(10000);
        
        
        printf("Programming all zeros attenuation coding\n");
        for (b = 0; b < ATTENCODES; b++) {
            data = 0;
            beamcode = b;

            //printf("B: %d data: %d BC: %d\n",b,data,beamcode);
            temp = write_data(&phasing_matrix, c, beamcode, data);
            temp = write_attenuators(&phasing_matrix, c, beamcode, data);

        }

        printf("Verifying all zero programming attenuation coding\n");
        for (b = 0; b < ATTENCODES; b++) {
            select_card(&phasing_matrix, c);
            select_beam_code(&phasing_matrix, b);
            usleep(10000);
            temp = verify_attenuators(&phasing_matrix, c, b, 0);
        }

        printf("Programming 1-to-1 attenuation coding no phase\n");
        for (b = 0; b < ATTENCODES; b++) {
            data = b;
            beamcode = b;

            //printf("B: %d data: %d BC: %d\n",b,data,beamcode);
            temp = write_data(&phasing_matrix, c, beamcode, 0);
            temp = write_attenuators(&phasing_matrix, c, beamcode, b);
        }
        printf("Verifying 1-to-1 programming attenuation coding\n");
        for (b = 0; b < ATTENCODES; b++) {
            select_card(&phasing_matrix, c);
            select_beam_code(&phasing_matrix, b);
            usleep(10000);
            temp = verify_attenuators(&phasing_matrix, c, b, b);
        }


        if (test_flag == -1) {
            exit(0);
        }

        sleep(10);


        if (verbose > 0) fprintf(stdout, "Measuring attens\n");
        last_collect = 0;
        current_collect = 0;
        for (b = 0; b < ATTENCODES; b++) {
            beamcode = b;
            temp = select_card(&phasing_matrix, c);
            select_beam_code(&phasing_matrix, beamcode);
            vna_button_command(":INIT1:IMM\r\n", 0, verbose);
            if (b == 0) sleep(1);

            usleep(wait_delay);

            take_data = 1;
            attempt = 0;
            while ((take_data) && (attempt < max_attempts)) {

                attempt++;
                vna_button_command(":CALC1:PAR1:SEL\r\n", 0, verbose);
                log_vna_data(":CALC1:DATA:FDAT?\r\n", atten_phase, b, verbose);
                vna_button_command(":CALC1:PAR2:SEL\r\n", 0, verbose);
                log_vna_data(":CALC1:DATA:FDAT?\r\n", atten_pwr_mag, b, verbose);
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
            //if (verbose > 1) printf("Freq %lf:  Phase 0:%lf Phase Max: %lf\n",freq[i],atten_phase[i][0],atten_phase[i][ATTENCODES-1]);
            if (verbose > 1)
                printf("Freq %lf:  Pwr 0:%lf Pwr Max: %lf\n", freq[i], atten_pwr_mag[i][0],
                       atten_pwr_mag[i][ATTENCODES - 1]);
            fwrite(&i, sizeof(int32), 1, attenfile);
            count = fwrite(atten_phase[i], sizeof(double), ATTENCODES, attenfile);
            count = fwrite(atten_pwr_mag[i], sizeof(double), ATTENCODES, attenfile);
            //printf("Freq index: %d Count: %d\n",i,count);
        }
        printf("Closing File\n");
        fclose(attenfile);
        c = -1;
//    if (verbose>0) fprintf(stdout,"Asking for another card:\n");
//    printf("Enter Next Card Number (CTRL-C to exit): ");
//    scanf("%d", &c);
    } // end of Card loop
}

