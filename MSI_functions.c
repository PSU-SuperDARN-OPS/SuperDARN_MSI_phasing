/* 
 * Calculate the timedelay needed for a given angle relative to boresite
 *   angle_degress: angle in degrees
 *   spacing_meters: antenna spacing in meters
 *   card: phasing card number numbered from 0. Card 0 is west most phasing card
 * Note:
 *   Assumes 16 cards in main array, numbered 0-15
 *   Assumes 4 cards in interf array, numbered 16-19
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

#include "include/MSI_functions.h"
#include "include/phasing_cards.h"
#include "include/pci_dio_120.h"
/* settings which I could probably move to an ini file */
uint32_t    MSI_phasecodes=8192;
int32_t    MSI_num_angles=24;
int32_t    MSI_max_angles=32;
int32_t    MSI_num_cards=20;
double     MSI_bm_sep_degrees=3.24;
double     MSI_spacing_meters=12.8016;
double     MSI_max_freq=20E6;
double     MSI_min_freq=8E6;
double     MSI_lo_freq=10E6;
double     MSI_hi_freq=16E6;
int32_t    MSI_max_freq_steps=128;
double     MSI_freq_window=0.25*1E6;
double     MSI_target_pwr_dB=-2.0;
double     MSI_target_tdelay0_nsecs=10.0;
double     MSI_tdelay_tolerance_nsec=3.0;
double     MSI_pwr_tolerance_dB=1.0;

char       ssh_userhost[128]="";

/* Hardwired stuff */

double MSI_timedelay_bits_nsecs[13]={
                         0.25,
                         0.45,
                         0.8,
                         1.5,
                         2.75,
                         5.0,
                         8.0,
                         15.0,
                         25.0,
                         45.0,
                         80.0,
                         140.0,
                         250.0
                       };

double MSI_atten_bits_dB[6]={
                         0.5,
                         1.0,
                         2.0,
                         4.0,
                         8.0,
                         16.0,
                       };


int32_t MSI_attencode(double target_dB) {
    int32_t attencode;
    double remain;
    int i;

    attencode = 0;
    remain = target_dB;

    for (i = 5; i >= 0; i--) {
        if ((remain - MSI_atten_bits_dB[i]) > 0.0) {
            attencode += (int32_t)pow(2, i);
            remain -= MSI_atten_bits_dB[i];
        }
    }
    return attencode;
}

int32_t MSI_phasecode(double target_nsec) {
    int32_t phasecode;
    double remain;
    int i;

    phasecode = 0;
    remain = target_nsec;

    for (i = 12; i >= 0; i--) {
        if ((remain - MSI_timedelay_bits_nsecs[i]) > 0.0) {
            phasecode += (int32_t)pow(2, i);
            remain -= MSI_timedelay_bits_nsecs[i];
        }
    }
    return phasecode;
}

double MSI_timedelay_needed(double angle_degrees,double spacing_meters,int32_t card) {
/*
 *  * *  angle from broadside (degrees)  spacing in meters
 *   * */
    double deltat = 0;
    double needed = 0;
    double c = 0.299792458; // meters per nanosecond
    int32_t antenna = -1;

    if (card > 15) {
        antenna = card - 10;
    } else {
        antenna = card;
    }
    deltat = (spacing_meters / c) * sin((fabs(angle_degrees) * 3.14159) / 180.0); //nanoseconds
    if (angle_degrees > 0) needed = antenna * deltat;
    if (angle_degrees < 0) needed = (15 - antenna) * deltat;
    if (needed < 0) {
        fprintf(stderr, "Error in Time Needed Calc: %lf %lf\n", needed, deltat);
    }
    return needed;
}

int MSI_dio_write_memory(struct DIO const *phasing_matrix, int code, int card, int phasecode, int attencode) {

    int rval_d, rval_a;
    int try = 3;
    int uwait = 10;

    if (code >= MSI_phasecodes) {
        fprintf(stderr, "Bad memory address: %d\n", code);
        return 1;
    }

    while (try > 0) {
        rval_d = write_data(phasing_matrix, card, code, phasecode);

        if (rval_d != 0) {
            fprintf(stderr, "Dio memory write data error, exiting\n");
            return rval_d;
        }

        rval_a = write_attenuators(phasing_matrix, card, code, attencode);

        if (rval_a != 0) {
            fprintf(stderr, "Dio memory write attenuator error, exiting\n");
            return rval_a;
        }

        usleep(uwait);

        rval_d = verify_data(phasing_matrix, card, code, phasecode);
        rval_a = verify_attenuators(phasing_matrix, card, code, attencode);

        if (rval_a != 0 || rval_d != 0) {
            fprintf(stderr, "Dio memory verify error, try again: %d, %d\n", rval_d, rval_a);

            uwait += 500;
            try--;
        } else {
            break;
        }
    }
    if (try <= 0) {
        fprintf(stderr, "Dio memory verify error, try again: %d, %d\n", rval_d, rval_a);
        return -1;
    }
    fflush(stdout);
    return 0;
}

int MSI_dio_verify_memory(int code,int rnum,int card, int phasecode,int attencode,int ssh_flag,int verbose){
    int temp, pci_handle, IRQ;
    unsigned int mmap_io_ptr, IOBASE;
    int rval_d, rval_a;
    int try = 3;
    int uwait = 10;

    verbose = 2;

    if (verbose > 2) fprintf(stdout, "Take Data: card:%d b:%d phasecode:%d attencode:%d\n", card, code, phasecode, attencode);

    if (code >= MSI_phasecodes) {
        fprintf(stderr, "Bad memory address: %d\n", code);
        return 1;
    }

    /* OPEN THE PLX9656 AND GET LOCAL BASE ADDRESSES */
    fprintf(stderr, "PLX9052 CONFIGURATION ********************\n");
    temp = init_pci_dio_120();
    IOBASE = mmap_io_ptr;
    if (temp == -1) {
        fprintf(stderr, "	PLX9052 configuration failed");
    } else {
        fprintf(stderr, "	PLX9052 configuration successful!\n");
    }
    printf("IOBASE=%x\n", IOBASE);

    while (try > 0) {
        rval_a = verify_attenuators(IOBASE, card, code, attencode);
        rval_d = verify_data(IOBASE, card, code, phasecode);

        if (rval_a != 0 || rval_d != 0) {
            fprintf(stderr, "Dio memory verify error, try again: %d, %d\n", rval_d, rval_a);

            uwait += 500;
            verbose = 2;
            try--;
        } else {
            break;
        }
    }
    if (try <= 0) {
        fprintf(stderr, "Dio memory verify error, try again: %d, %d\n", rval_d, rval_a);
        return -1;
    }
    fflush(stdout);
    return 0;
}
