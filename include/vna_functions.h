
#ifndef __VNA_FUNCTIONS_H__
#define __VNA_FUNCTIONS_H__

#define VNA_MIN    5E6
#define VNA_MAX   25E6
#define VNA_FREQS  1500

struct VNA{
    char * host_ip;
    int host_port;
    int socket;
};

void init_vna(char * host_ip, int host_port);
void calibrate_vna(char * freq_start, char * freq_stop, char * freq_steps);
int log_vna_data(char *command, double **array, int b, int verbose);
int vna_button_command(char *command, int delay_ms, int verbose);
int take_data(int b, struct DIO const *phasing_matrix, int c, int p, int a, double **pwr_mag, double **phase,
              double **tdelay, int wait_ms, int ssh_flag, int verbose, double target_tdelay, double target_pwr);

#endif // end __VNA_FUNCTIONS_H__
