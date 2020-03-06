#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "include/MSI_functions.h"
#include "include/vna_functions.h"
#include "include/utils.h"

int32_t VNA_triggers=4;
int32_t VNA_wait_delay_ms=10;
int32_t VNA_min_nave=3;

extern int32_t MSI_phasecodes;

struct VNA vna;

void init_vna(char * host_ip, int host_port) {
    char output[40], strout[40];
    ssize_t rval;
    vna.host_ip = host_ip;
    vna.host_port = host_port;

    vna.socket = opentcpsock(vna.host_ip, vna.host_port);

    if(vna.socket < 0) {
        printf("Error: VNA connection failed\n");
        exit(-1);
    }

    rval = read(vna.socket, &output, sizeof(char) * 10);
    fprintf(stdout, "Initial Output Length: %ld\n", rval);
    strcpy(strout, "");
    strncat(strout, output, rval);
    fprintf(stdout, "Initial Output String: %s\n", strout);
}

void calibrate_vna(char * freq_start, char * freq_stop, char * freq_steps) {
    char command[80];
    int verbose = 3;

    vna_button_command(":SYST:PRES\r\n", 0, verbose);
    sprintf(command, ":SENS1:FREQ:STAR %s\r\n", freq_start);
    vna_button_command(command, 0, verbose);
    sprintf(command, ":SENS1:FREQ:STOP %s\r\n", freq_stop);
    vna_button_command(command, 0, verbose);
    sprintf(command, ":SENS1:SWE:POIN %s\r\n", freq_steps);
    vna_button_command(command, 0, verbose);
    vna_button_command(":CALC1:PAR:COUN 2\r\n", 0, verbose);
    vna_button_command(":CALC1:PAR1:SEL\r\n", 0, verbose);
    vna_button_command(":CALC1:PAR1:DEF S12\r\n", 0, verbose);
    vna_button_command(":CALC1:FORM UPH\r\n", 0, verbose);
    vna_button_command(":CALC1:PAR2:SEL\r\n", 0, verbose);
    vna_button_command(":CALC1:PAR2:DEF S12\r\n", 0, verbose);
    vna_button_command(":CALC1:FORM MLOG\r\n", 0, verbose);
    vna_button_command(":SENS1:AVER OFF\r\n", 0, verbose);
    vna_button_command(":SENS1:AVER:COUN 4\r\n", 0, verbose);
    vna_button_command(":SENS1:AVER:CLE\r\n", 0, verbose);
    vna_button_command(":INIT1:CONT OFF\r\n", 0, verbose);

    printf("\n\n\7\7Calibrate Network Analyzer for S12,S21\n");
    mypause();
    vna_button_command(":SENS1:CORR:COLL:METH:THRU 1,2\r\n", 0, verbose);
    sleep(1);
    vna_button_command(":SENS1:CORR:COLL:THRU 1,2\r\n", 0, verbose);
    printf("  Doing S1,2 Calibration..wait 4 seconds\n");
    sleep(4);

    vna_button_command(":SENS1:CORR:COLL:METH:THRU 2,1\r\n", 0, verbose);
    sleep(1);
    vna_button_command(":SENS1:CORR:COLL:THRU 2,1\r\n", 0, verbose);
    printf("  Doing S2,1 Calibration..wait 4 seconds\n");
    sleep(4);
    vna_button_command(":SENS1:CORR:COLL:SAVE\r\n", 0, verbose);
}

int log_vna_data(char *command, double **array, int b, int verbose) {
  int32_t count, rval, sample_count;
  char output[10]="";
  char command2[80];
  char cmd_str[80],prompt_str[10],data_str[1000];
  double base,exp;
  int32_t cr,lf;
      strcpy(command2,command);
      if (verbose>2) printf("%d Command: %s\n",(int) strlen(command2),command2);
      write(vna.socket, &command2, sizeof(char)*strlen(command2));
      cr=0;
      lf=0;
      count=0;
      if (verbose>2) fprintf(stdout,"Command Output String::\n");
      strcpy(cmd_str,"");
      while((cr==0) || (lf==0)){
        rval=read(vna.socket, &output, sizeof(char)*1);
#ifdef __QNX__
        if (rval<1) usleep(1000);
#else
        if (rval<1) {
          usleep(10);
  
        }
#endif
        if (output[0]==13) {
          cr++;
          continue;
        }
        if (output[0]==10) {
          lf++;
          continue;
        }
        count+=rval;
        strncat(cmd_str,output,rval);
        if (verbose>2) fprintf(stdout,"%c",output[0]);
      }
      if (verbose>2) printf("Processing Data\n");

      cr=0;
      lf=0;
      count=0;
      sample_count=0;
      if (verbose>2) fprintf(stdout,"\nData Output String::\n");
      strcpy(data_str,"");
      if (verbose>2) fprintf(stdout,"%d: ",sample_count);
      while((cr==0) || (lf==0)){
        rval=read(vna.socket, &output, sizeof(char)*1);
        if (output[0]==13) {
          cr++;
          continue;
        }
        if (output[0]==10) {
         lf++;
          continue;
        }
        if(output[0]==',') {
             base=0;
             exp=0;
             if((sample_count % 2) == 0) {
               if (sample_count/2 >=VNA_FREQS) {
                 printf("ERROR: too many samples... aborting\n");
                 exit(-1);
               }
               base=atof(strtok(data_str, "E"));
               exp=atof(strtok(NULL, "E"));
               array[sample_count/2][b]=base*pow(10,exp);
               if (verbose>2) fprintf(stdout,"%d ::  %s  ::  %lf , %lf :: %g",sample_count/2,data_str,base,exp,array[sample_count/2][b]);
             }
             sample_count++;
             if (verbose>2) fprintf(stdout,"\n%d: ",sample_count);
             strcpy(data_str,"");
        } else {
             strncat(data_str,output,rval);
        }
      }
      if((sample_count % 2) == 0) {
        if (sample_count/2 >=VNA_FREQS) {
          printf("ERROR: too many samples... aborting\n");
          exit(-1);
        }
        array[sample_count/2][b]=atof(data_str);
        if (verbose>2) fprintf(stdout,"%s  ::  %lf",data_str,array[sample_count/2][b]);
      }
      sample_count++;
      strcpy(data_str,"");
      if (verbose>2) fprintf(stdout,"\nSamples: %d\n",sample_count/2);
      if (verbose>2) fprintf(stdout,"\nPrompt String::\n");
      while(output[0]!='>'){
        rval=read(vna.socket, &output, sizeof(char)*1);
#ifdef __QNX__
        if (rval<1) usleep(1000);
#else
        if (rval<1) usleep(10);
#endif
        strncat(prompt_str,output,rval);
        if (verbose>2) fprintf(stdout,"%c",output[0]);
      }
  return 0;
}

int vna_button_command(char *command, int delay_ms, int verbose) {
  int32_t count,rval;
  char output[10]="";
  char command2[80];
  char prompt_str[80];
/*
 * *  Process Command String with No feedback 
 * */
  strcpy(command2,command);
  if (verbose>2) fprintf(stdout,"%d Command: %s\n",(int) strlen(command2),command2);
  write(vna.socket, &command2, sizeof(char)*strlen(command2));
  count=0;
  if (verbose>2) fprintf(stdout,"\nPrompt String::\n");
  while(output[0]!='>'){
    rval=read(vna.socket, &output, sizeof(char)*1);
    strncat(prompt_str,output,rval);
    if (verbose>2) fprintf(stdout,"%c",output[0]);
    count++;
  }
  if (verbose>2) fprintf(stdout,"Command is done\n");
  fflush(stdout);
  return 0;
}




