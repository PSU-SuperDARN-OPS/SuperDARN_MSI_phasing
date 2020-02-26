
#ifndef __VNA_FUNCTIONS_H__
#define __VNA_FUNCTIONS_H__

#define VNA_MIN    5E6
#define VNA_MAX   25E6
#define VNA_FREQS  1500

int mlog_data_command(int sock,char *command,double *array[VNA_FREQS],int b,int verbose);
int button_command(int sock, char *command, int delay_ms, int verbose);
int take_data(int sock, int b, struct DIO const *phasing_matrix, int c, int p, int a,
              double **pwr_mag, double **phase, double **tdelay,
              int wait_ms, int ssh_flag, int verbose, double target_tdelay, double target_pwr);

#endif // end __VNA_FUNCTIONS_H__
