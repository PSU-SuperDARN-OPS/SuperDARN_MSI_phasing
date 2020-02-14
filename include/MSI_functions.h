
#ifndef __MSI_HEADER__
#define __MSI_HEADER__
#include <stdint.h>

int32_t MSI_attencode(double target_dB);
int32_t MSI_phasecode(double target_nsec);
double MSI_timedelay_needed(double angle_degrees,double spacing_meter,int32_t card);
int MSI_dio_write_memory(int b,int rnum,int c, int p,int a,int ssh_flag,int verbose);
int MSI_dio_verify_memory(int memloc,int rnum,int c, int p,int a,int ssh_flag,int verbose);


#endif
