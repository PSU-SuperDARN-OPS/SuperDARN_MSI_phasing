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
#include <sys/io.h>

#include "include/registers.h"
#include "include/_open_PLX9050.h"

#define SWITCHES 0
#define ATTEN    1
#define READ     0
#define WRITE    1
#define ON       1
#define OFF      0

#define NEW_PMAT 1

#define CARDS 200
#define PHASECODES 8192
#define ATTENCODES 64
//#define PHASECODES  64 
#define FREQS 1500
#define _QUICK_

int stupid_flag = 0;
int setup_flag = 1;
int test_flag = -1000;
int sock = -1;
int verbose = 2;
//char *hostip="192.168.1.2";
//char *hostip="209.114.113.119";
char *hostip = "137.229.27.122";
//char *hostip="67.59.83.38";
char *file_prefix = "phasing_cal";
char *file_ext = ".dat";
char filename[120];
char *dir = "/data/calibrations/";
FILE *calfile = NULL;
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
    double *phase[FREQS], *pwr_mag[FREQS];
    double freq[FREQS];
    double pd_old, pd_new, phase_diff = 0.0;
    int rval, count, sample_count, fail, cr, lf;
    int ii, i = 0, c = 31, data = 0, index = 0, wait_delay = 10;
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

    /*struct _clockperiod new, old;
    struct timespec start_p, stop_p, start, stop, nsleep;

    // SET THE SYSTEM CLOCK RESOLUTION AND GET THE START TIME OF THIS PROCESS
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
    */

    /* OPEN THE PLX9656 AND GET LOCAL BASE ADDRESSES */
    fprintf(stderr, "PLX9052 CONFIGURATION ********************\n");
    //clock_gettime(CLOCK_REALTIME, &start);
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
    outb(IOBASE + CNTRL_GRP_0, 0x80);
    outb(IOBASE + CNTRL_GRP_1, 0x80);
    outb(IOBASE + CNTRL_GRP_2, 0x80);
    outb(IOBASE + CNTRL_GRP_3, 0x80);
    outb(IOBASE + CNTRL_GRP_4, 0x80);
    outb(IOBASE + cntrl1, 0x80);
    outb(IOBASE + PA_GRP_0, 0x00);
    outb(IOBASE + PB_GRP_0, 0x00);
    outb(IOBASE + PC_GRP_0, 0x00);
    outb(IOBASE + PA_GRP_1, 0x00);
    outb(IOBASE + PB_GRP_1, 0x00);
    outb(IOBASE + PC_GRP_1, 0x00);
    outb(IOBASE + PA_GRP_2, 0x00);
    outb(IOBASE + PB_GRP_2, 0x00);
    outb(IOBASE + PC_GRP_2, 0x00);
    outb(IOBASE + PA_GRP_3, 0x00);
    outb(IOBASE + PB_GRP_3, 0x00);
    outb(IOBASE + PC_GRP_3, 0x00);
    outb(IOBASE + PA_GRP_4, 0x00);
    outb(IOBASE + PB_GRP_4, 0x00);
    outb(IOBASE + PC_GRP_4, 0x00);
    outb(IOBASE + CNTRL_GRP_0, 0x00);
    outb(IOBASE + CNTRL_GRP_1, 0x00);
    outb(IOBASE + CNTRL_GRP_2, 0x00);
    outb(IOBASE + CNTRL_GRP_3, 0x00);
    outb(IOBASE + CNTRL_GRP_4, 0x00);
    //temp=in8(IOBASE+portC1);
    //temp=temp & 0x0f;
    //printf("input on group 1, port c is %x\n", temp);
    printf("Testing DIO outputs all ports all bits 5 seconds HIGH 5 seconds low repeated\n");
    while (1) {
        sleep(5);
        printf("Outputs High\n");
        outb(IOBASE + PA_GRP_0, 0xff);
        outb(IOBASE + PB_GRP_0, 0xff);
        outb(IOBASE + PC_GRP_0, 0xff);
        outb(IOBASE + PA_GRP_1, 0xff);
        outb(IOBASE + PB_GRP_1, 0xff);
        outb(IOBASE + PC_GRP_1, 0xff);
        outb(IOBASE + PA_GRP_2, 0xff);
        outb(IOBASE + PB_GRP_2, 0xff);
        outb(IOBASE + PC_GRP_2, 0xff);
        outb(IOBASE + PA_GRP_3, 0xff);
        outb(IOBASE + PB_GRP_3, 0xff);
        outb(IOBASE + PC_GRP_3, 0xff);
        outb(IOBASE + PA_GRP_4, 0xff);
        outb(IOBASE + PB_GRP_4, 0xff);
        outb(IOBASE + PC_GRP_4, 0xff);
        sleep(5);
        printf("Outputs Low\n");
        outb(IOBASE + PA_GRP_0, 0x00);
        outb(IOBASE + PB_GRP_0, 0x00);
        outb(IOBASE + PC_GRP_0, 0x00);
        outb(IOBASE + PA_GRP_1, 0x00);
        outb(IOBASE + PB_GRP_1, 0x00);
        outb(IOBASE + PC_GRP_1, 0x00);
        outb(IOBASE + PA_GRP_2, 0x00);
        outb(IOBASE + PB_GRP_2, 0x00);
        outb(IOBASE + PC_GRP_2, 0x00);
        outb(IOBASE + PA_GRP_3, 0x00);
        outb(IOBASE + PB_GRP_3, 0x00);
        outb(IOBASE + PC_GRP_3, 0x00);
        outb(IOBASE + PA_GRP_4, 0x00);
        outb(IOBASE + PB_GRP_4, 0x00);
        outb(IOBASE + PC_GRP_4, 0x00);
    }

}




