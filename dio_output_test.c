
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "include/pci_dio_120.h"
#include "external/APCI/apcilib/apcilib.h"


int main(int argc, char **argv) {

    uint8_t data = 0;

    /* OPEN THE PLX9656 AND GET LOCAL BASE ADDRESSES */
    fprintf(stderr, "PLX9052 CONFIGURATION ********************\n");
    //clock_gettime(CLOCK_REALTIME, &start);
    init_pci_dio_120();


    /* INITIALIZE THE CARD FOR PROPER IO */
    // GROUP 0 - PortA=output, PortB=output, PortClo=output, PortChi=output
    data = read_pci_dio_120(3);
    printf("input %x\n", data);
    write_pci_dio_120(3, 0x80);
    data = read_pci_dio_120(3);
    printf("input %x\n", data);
    write_pci_dio_120(CNTRL_GRP_0, 0x80);
    write_pci_dio_120(CNTRL_GRP_1, 0x80);
    write_pci_dio_120(CNTRL_GRP_2, 0x80);
    write_pci_dio_120(CNTRL_GRP_3, 0x80);
    write_pci_dio_120(CNTRL_GRP_4, 0x80);
    write_pci_dio_120(PA_GRP_0, 0x00);
    write_pci_dio_120(PB_GRP_0, 0x00);
    write_pci_dio_120(PC_GRP_0, 0x00);
    write_pci_dio_120(PA_GRP_1, 0x00);
    write_pci_dio_120(PB_GRP_1, 0x00);
    write_pci_dio_120(PC_GRP_1, 0x00);
    write_pci_dio_120(PA_GRP_2, 0x00);
    write_pci_dio_120(PB_GRP_2, 0x00);
    write_pci_dio_120(PC_GRP_2, 0x00);
    write_pci_dio_120(PA_GRP_3, 0x00);
    write_pci_dio_120(PB_GRP_3, 0x00);
    write_pci_dio_120(PC_GRP_3, 0x00);
    write_pci_dio_120(PA_GRP_4, 0x00);
    write_pci_dio_120(PB_GRP_4, 0x00);
    write_pci_dio_120(PC_GRP_4, 0x00);
    write_pci_dio_120(CNTRL_GRP_0, 0x00);
    write_pci_dio_120(CNTRL_GRP_1, 0x00);
    write_pci_dio_120(CNTRL_GRP_2, 0x00);
    write_pci_dio_120(CNTRL_GRP_3, 0x00);
    write_pci_dio_120(CNTRL_GRP_4, 0x00);

    printf("Testing DIO outputs all ports all bits 5 seconds HIGH 5 seconds low repeated\n");
    while (1) {
        sleep(5);
        printf("Outputs High\n");
        write_pci_dio_120(PA_GRP_0, 0xff);
        write_pci_dio_120(PB_GRP_0, 0xff);
        write_pci_dio_120(PC_GRP_0, 0xff);
        write_pci_dio_120(PA_GRP_1, 0xff);
        write_pci_dio_120(PB_GRP_1, 0xff);
        write_pci_dio_120(PC_GRP_1, 0xff);
        write_pci_dio_120(PA_GRP_2, 0xff);
        write_pci_dio_120(PB_GRP_2, 0xff);
        write_pci_dio_120(PC_GRP_2, 0xff);
        write_pci_dio_120(PA_GRP_3, 0xff);
        write_pci_dio_120(PB_GRP_3, 0xff);
        write_pci_dio_120(PC_GRP_3, 0xff);
        write_pci_dio_120(PA_GRP_4, 0xff);
        write_pci_dio_120(PB_GRP_4, 0xff);
        write_pci_dio_120(PC_GRP_4, 0xff);
        sleep(5);
        printf("Outputs Low\n");
        write_pci_dio_120(PA_GRP_0, 0x00);
        write_pci_dio_120(PB_GRP_0, 0x00);
        write_pci_dio_120(PC_GRP_0, 0x00);
        write_pci_dio_120(PA_GRP_1, 0x00);
        write_pci_dio_120(PB_GRP_1, 0x00);
        write_pci_dio_120(PC_GRP_1, 0x00);
        write_pci_dio_120(PA_GRP_2, 0x00);
        write_pci_dio_120(PB_GRP_2, 0x00);
        write_pci_dio_120(PC_GRP_2, 0x00);
        write_pci_dio_120(PA_GRP_3, 0x00);
        write_pci_dio_120(PB_GRP_3, 0x00);
        write_pci_dio_120(PC_GRP_3, 0x00);
        write_pci_dio_120(PA_GRP_4, 0x00);
        write_pci_dio_120(PB_GRP_4, 0x00);
        write_pci_dio_120(PC_GRP_4, 0x00);
    } 

}




