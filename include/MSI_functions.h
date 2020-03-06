
#ifndef __MSI_HEADER__
#define __MSI_HEADER__
#include <stdint.h>
#include "phasing_cards.h"

int32_t MSI_attencode(double target_dB);
int32_t MSI_phasecode(double target_nsec);
double MSI_timedelay_needed(double angle_degrees,double spacing_meter,int32_t card);
int MSI_dio_write_memory(struct DIO const *phasing_matrix, int code, int card, int phasecode, int attencode);
int MSI_dio_verify_memory(int code,int rnum,int card, int phasecode,int attencode,int ssh_flag,int verbose);

int take_data(int b, struct DIO const *phasing_matrix, int c, int p, int a, double **pwr_mag, double **phase,
              double **tdelay, int wait_ms, int ssh_flag, int verbose, double target_tdelay, double target_pwr);

#endif
