//
// Created by bgklug on 3/25/20.
//

#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <signal.h>

#include "include/vna_functions.h"
#include "include/MSI_functions.h"
#include "include/utils.h"
#include "include/pci_dio_120.h"

/* variables defined elsewhere */
extern int32_t MSI_max_angles;
extern uint32_t MSI_phasecodes;
extern int32_t MSI_num_angles;
extern double MSI_bm_sep_degrees;
extern double MSI_spacing_meters;
extern double MSI_max_freq;
extern double MSI_min_freq;
extern double MSI_lo_freq;
extern double MSI_hi_freq;
extern int32_t MSI_max_freq_steps;
extern double MSI_freq_window;
extern double MSI_target_pwr_dB;
extern double MSI_target_tdelay0_nsecs;
extern double MSI_tdelay_tolerance_nsec;
extern double MSI_pwr_tolerance_dB;

extern int32_t VNA_triggers;
extern int32_t VNA_min_nave;

int main(){
    char filename[512] = "";
    char dirstub[256] = "/data/cal/ice";
    char radar_name[16] = "ice";
    int c = 6;
    unsigned int f,a;
    FILE *beamcodefile = NULL;
    int verbose = 2;

    double beam_highest_time0_nsec, beam_lowest_pwr_dB, beam_middle;
    uint32_t num_beam_freqs, num_beam_angles, num_beam_steps;
    double *beam_freqs = NULL;
    double *beam_angles = NULL;
    double *beam_requested_delay = NULL;
    double *beam_needed_delay = NULL;
    double *beam_freq_lo = NULL;
    double *beam_freq_hi = NULL;
    double *beam_freq_center = NULL;
    double *beam_pwr_dB = NULL;
    double *beam_tdelay_nsec = NULL;
    int32_t *beam_attencode = NULL;
    int32_t *beam_phasecode = NULL;
    int32_t beam_freq_index;

    /* Load the beamtable file for this card*/
    sprintf(filename, "%s/beamcodes_cal_%s_%02d.dat", dirstub, radar_name, c);
    beamcodefile = fopen(filename, "r");
    if (beamcodefile != NULL) {
        if (verbose > -1) fprintf(stdout, "    Opened: %s\n", filename);
        fread(&beam_highest_time0_nsec, sizeof(double), 1, beamcodefile);
        printf("Longest delay 0: %f ns\n", beam_highest_time0_nsec);
        if (beam_highest_time0_nsec != MSI_target_tdelay0_nsecs) {
            fprintf(stderr, "Error:: Card %d Target time0 mismatch: %lf  %lf\n", c, beam_highest_time0_nsec,
                    MSI_target_tdelay0_nsecs);
//                 exit(-1);
        }

        fread(&beam_lowest_pwr_dB, sizeof(double), 1, beamcodefile);
        printf("Lowest power: %f dB\n", beam_lowest_pwr_dB);
        if (beam_lowest_pwr_dB != MSI_target_pwr_dB) {
            fprintf(stderr, "Error:: Card %d Target pwr  mismatch: %lf  %lf\n", c, beam_lowest_pwr_dB,
                    MSI_target_pwr_dB);
            exit(-1);
        }

        fread(&num_beam_freqs, sizeof(int), 1, beamcodefile);
        printf("Number of beam frequencies: %d\n", num_beam_freqs);

        if (beam_freqs != NULL) free(beam_freqs);
        beam_freqs = calloc(num_beam_freqs, sizeof(double));
        fread(beam_freqs, sizeof(double), num_beam_freqs, beamcodefile);
        fread(&num_beam_angles, sizeof(int32_t), 1, beamcodefile);
        printf("Number of beam angles: %d \n", num_beam_angles);
        fread(&beam_middle, sizeof(double), 1, beamcodefile);

        if (beam_angles != NULL) free(beam_angles);
        beam_angles = calloc(num_beam_angles, sizeof(double));
        fread(beam_angles, sizeof(double), num_beam_angles, beamcodefile);

        if (beam_requested_delay != NULL) free(beam_requested_delay);
        beam_requested_delay = calloc(num_beam_angles, sizeof(double));
        fread(beam_requested_delay, sizeof(double), num_beam_angles, beamcodefile);

        if (beam_needed_delay != NULL) free(beam_needed_delay);
        beam_needed_delay = calloc(num_beam_angles, sizeof(double));
        fread(beam_needed_delay, sizeof(double), num_beam_angles, beamcodefile);
        fread(&num_beam_steps, sizeof(int32_t), 1, beamcodefile);

        if (beam_freq_lo != NULL) free(beam_freq_lo);
        beam_freq_lo = calloc(num_beam_steps + 1, sizeof(double));

        if (beam_freq_hi != NULL) free(beam_freq_hi);
        beam_freq_hi = calloc(num_beam_steps + 1, sizeof(double));

        if (beam_freq_center != NULL) free(beam_freq_center);
        beam_freq_center = calloc(num_beam_steps + 1, sizeof(double));

        if (beam_pwr_dB != NULL) free(beam_pwr_dB);
        beam_pwr_dB = calloc(num_beam_angles * (num_beam_steps + 1), sizeof(double));

        if (beam_tdelay_nsec != NULL) free(beam_tdelay_nsec);
        beam_tdelay_nsec = calloc(num_beam_angles * (num_beam_steps + 1), sizeof(double));

        if (beam_phasecode != NULL) free(beam_phasecode);
        beam_phasecode = calloc(num_beam_angles * (num_beam_steps + 1), sizeof(int32_t));

        if (beam_attencode != NULL) free(beam_attencode);
        beam_attencode = calloc(num_beam_angles * (num_beam_steps + 1), sizeof(int32_t));

        for (f = 0; f <= num_beam_steps; f++) {
            fread(&beam_freq_index, sizeof(int32_t), 1, beamcodefile);
            if (f != beam_freq_index) {
                fprintf(stderr, "Error:: Read file error! %d %d\n", f, beam_freq_index);
                exit(-1);
            }
            fread(&beam_freq_lo[f], sizeof(double), 1, beamcodefile);
            fread(&beam_freq_hi[f], sizeof(double), 1, beamcodefile);
            beam_freq_center[f] = (beam_freq_lo[f] + beam_freq_hi[f]) / 2.0;


            for (a = 0; a < num_beam_angles; a++)
                fread(&beam_pwr_dB[(f * num_beam_angles) + a], sizeof(double), 1, beamcodefile);
            for (a = 0; a < num_beam_angles; a++)
                fread(&beam_tdelay_nsec[(f * num_beam_angles) + a], sizeof(double), 1, beamcodefile);
            for (a = 0; a < num_beam_angles; a++)
                fread(&beam_attencode[(f * num_beam_angles) + a], sizeof(int32_t), 1, beamcodefile);
            for (a = 0; a < num_beam_angles; a++)
                fread(&beam_phasecode[(f * num_beam_angles) + a], sizeof(int32_t), 1, beamcodefile);

            printf("Frequency step %2d \n", f);
            printf("Center Frequency: %.2f MHz\n", (beam_freq_center[f] / 1000000));
            printf("Frequency Range: %.2f - %.2f Mhz \n", (beam_freq_lo[f] / 1000000), (beam_freq_hi[f] / 1000000));
            printf("Angle | Phasecode | Delay (ns) | Attencode | Power (dB) \n");
            for (a = 0; a < num_beam_angles; a++) {
                printf("%5d | %9d | %10.2f | %9d | %.2f \n", a, beam_phasecode[(f * num_beam_angles) + a],
                        beam_tdelay_nsec[(f * num_beam_angles) + a], beam_attencode[(f * num_beam_angles) + a],
                        beam_pwr_dB[(f * num_beam_angles) + a]);
            }


        }

        fclose(beamcodefile);
    } else {
        fprintf(stdout, "    Warning::  Failed to Open: %s\n", filename);
        exit(-1);
    }

    return 0;
}
