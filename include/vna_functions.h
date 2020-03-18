
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

void vna_trigger_single(int verbose);
void vna_get_data(int trace, double ** data_ptr, int b, int verbose);


#endif // end __VNA_FUNCTIONS_H__
