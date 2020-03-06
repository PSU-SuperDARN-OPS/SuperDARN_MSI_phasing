//
// Created by bgklug on 3/5/20.
//

#include <stdio.h>

#include "include/vna_functions.h"


int main() {
    char *hostip = "192.168.1.2";
    int port = 23;

    char freq_start[10] = "8E6";
    char freq_stop[10] = "20E6";
    char freq_steps[10] = "201";

    printf("Initializing VNA\n");
    init_vna(hostip, port);

    printf("Calibrating VNA\n");
    calibrate_vna(freq_start, freq_stop, freq_steps);

    return 0;
}
