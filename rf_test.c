//
// Created by bgklug on 8/8/19.
//

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

#ifdef __QNX__
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#endif

#include "include/registers.h"
#include "include/common_functions.h"
#include "include/utils.h"
#include "include/_open_PLX9050.h"

#define FREQS 1500

int main(int argc, char **argv) {
    int stupid_flag = 0;
    int setup_flag = 1;
    int test_flag = -1000;
    char output[40], strout[40];
    //char cmd_str[80], prompt_str[10], data_str[1000];
    double *phase[FREQS], *pwr_mag[FREQS];
    double freq[FREQS];
    double pd_old, pd_new, phase_diff = 0.0;
    int32_t rval, count, sample_count, fail, cr, lf;
    int32_t ii, i = 0, c = 31, data = 0, index = 0, wait_delay_ms = 30;
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
    unsigned int mmap_io_ptr, IOBASE, CLOCK_RES;
    float time;
//#ifdef __QNX__
    struct _clockperiod new, old;
    struct timespec start_p, stop_p, start, stop, nsleep;

    if (argc < 2) {
        fprintf(stderr, "%s: invoke with radar number (1 or 2 or 3)\n", argv[0]);
        fflush(stderr);
        exit(0);
    }
    if (argc == 3) {
        if (atoi(argv[2]) == 0) setup_flag = 0;
        else setup_flag = 1;
    } else {
        test_flag = -1000;
    }
    if (argc == 4) {
        test_flag = atoi(argv[3]);
        c = atoi(argv[2]);
    } else {
        test_flag = -1000;
    }
    radar = atoi(argv[1]);
    printf("Radar: %d Card: %d\n", radar, c);
    printf("Test flag: %d\n", test_flag);
    switch (radar) {
        case 1:
            portC0 = PC_GRP_0;
            portC1 = PC_GRP_1;
            portB0 = PB_GRP_0;
            portB1 = PB_GRP_1;
            portA0 = PA_GRP_0;
            portA1 = PA_GRP_1;
            cntrl0 = CNTRL_GRP_0;
            cntrl1 = CNTRL_GRP_1;
            break;
        case 2:
            portC0 = PC_GRP_2;
            portC1 = PC_GRP_3;
            portB0 = PB_GRP_2;
            portB1 = PB_GRP_3;
            portA0 = PA_GRP_2;
            portA1 = PA_GRP_3;
            cntrl0 = CNTRL_GRP_2;
            cntrl1 = CNTRL_GRP_3;
            break;
        case 3:
            portC0 = PC_GRP_4;
            portC1 = PC_GRP_3;
            portB0 = PB_GRP_4;
            portB1 = PB_GRP_3;
            portA0 = PA_GRP_4;
            portA1 = PA_GRP_3;
            cntrl0 = CNTRL_GRP_4;
            cntrl1 = CNTRL_GRP_3;
            break;
        default:
            fprintf(stderr, "Invalid radar number %d", radar);
            exit(-1);
    }
    /* SET THE SYSTEM CLOCK RESOLUTION AND GET THE START TIME OF THIS PROCESS */
    new.nsec = 10000;
    new.fract = 0;
    temp = ClockPeriod(CLOCK_REALTIME, &new, 0, 0);
    if (temp == -1) {
        perror("Unable to change system clock resolution");
    }
    temp = clock_gettime(CLOCK_REALTIME, &start_p);
    if (temp == -1) {
        perror("Unable to read sytem time");
    }
    temp = ClockPeriod(CLOCK_REALTIME, 0, &old, 0);
    CLOCK_RES = old.nsec;
    printf("CLOCK_RES: %d\n", CLOCK_RES);
    /* OPEN THE PLX9656 AND GET LOCAL BASE ADDRESSES */
    fprintf(stderr, "PLX9052 CONFIGURATION ********************\n");
    clock_gettime(CLOCK_REALTIME, &start);
    temp = _open_PLX9052(&pci_handle, &mmap_io_ptr, &IRQ, 1);
    IOBASE = mmap_io_ptr;
    if (temp == -1) {
        fprintf(stderr, "	PLX9052 configuration failed");
    } else {
        fprintf(stderr, "	PLX9052 configuration successful!\n");
    }
    printf("IOBASE=%x\n", IOBASE);
    /* INITIALIZE THE CARD FOR PROPER IO */
    // GROUP 0 - PortA=output, PortB=output, PortClo=output, PortChi=output
    out8(IOBASE + cntrl0, 0x80);
    // GROUP 1 - PortAinput, PortB=input, PortClo=input, PortChi=output
    out8(IOBASE + cntrl1, 0x93);
    out8(IOBASE + portA0, 0x00);
    out8(IOBASE + portB0, 0x00);
    out8(IOBASE + portC0, 0x00);
    out8(IOBASE + portA1, 0x00);
    out8(IOBASE + portB1, 0x00);
    out8(IOBASE + cntrl0, 0x00);
    out8(IOBASE + cntrl1, 0x13);
    temp = in8(IOBASE + portC1);
    temp = temp & 0x0f;
    printf("input on group 1, port c is %x\n", temp);
    select_card(IOBASE, c, radar);
//#endif
    if (test_flag == -3) {
        while (1) {
            for (i = 0; i <= 12; i = i + 1) {
                if (i == -1) b = 0;
                if (i == 0) b = 1;
                if (i > 0) b = 1 << i;
                b = b | 0x200;
                printf("Selecting Beamcode: %d 0x%x\n", b, b);
                beam_code(IOBASE, b, 1);
                sleep(1);
            }
        }
    }
}