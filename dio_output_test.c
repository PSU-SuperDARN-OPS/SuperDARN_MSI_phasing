
#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
//#include <asm/io.h>

#include "include/registers.h"
#include "include/_open_PLX9050.h"
#include "external/APCI/apcilib/apcilib.h"


int main(int argc, char **argv) {
    int temp;
    int mmap_io_ptr, IOBASE;
    __u8 data = 0;

    /* OPEN THE PLX9656 AND GET LOCAL BASE ADDRESSES */
    fprintf(stderr, "PLX9052 CONFIGURATION ********************\n");
    //clock_gettime(CLOCK_REALTIME, &start);
    temp = _open_PLX9052(&mmap_io_ptr);
    IOBASE = mmap_io_ptr;
    if (temp == -1) {
        fprintf(stderr, "	PLX9052 configuration failed\n");
    } else {
        fprintf(stderr, "	PLX9052 configuration successful!\n");
    }
    printf("IOBASE=%x\n", IOBASE);
    /* INITIALIZE THE CARD FOR PROPER IO */
    // GROUP 0 - PortA=output, PortB=output, PortClo=output, PortChi=output
    temp = apci_read8(IOBASE, 1, 2, 3, &data);
    printf("input %x\n", data);
    temp = apci_write8(IOBASE,1, 2, 3, 0x80);
    temp = apci_read8(IOBASE, 1, 2, 3, &data);
    printf("input %x\n", data);
   /* outb(IOBASE + CNTRL_GRP_0, 0x80);
    outb(IOBASE + CNTRL_GRP_1, 0x80);
    outb(IOBASE + CNTRL_GRP_2, 0x80);
    outb(IOBASE + CNTRL_GRP_3, 0x80);
    outb(IOBASE + CNTRL_GRP_4, 0x80);
    outb(IOBASE + PA_GRP_0, 0x00);
    outb(IOBASE + PB_GRP_0, 0x00);
    outb(IOBASE + PC_GRP_0, 0x00);
    outb(IOBASE + PA_GRP_1, 0x00);
    outb(IOBASE + PB_GRP_1, 0x00);
    outb(IOBASE + PC_GRP_1, 0x00);
    outb(IOBASE + PA_GRP_2, 0x00);
    outb(IOBASE + PB_GRP_2, 0x00);
    outb(IOBASE + PC_GRP_2, 0x00);
    outb(IOBASE + PA_GRP_3, 0x00);
    outb(IOBASE + PB_GRP_3, 0x00);
    outb(IOBASE + PC_GRP_3, 0x00);
    outb(IOBASE + PA_GRP_4, 0x00);
    outb(IOBASE + PB_GRP_4, 0x00);
    outb(IOBASE + PC_GRP_4, 0x00);
    outb(IOBASE + CNTRL_GRP_0, 0x00);
    outb(IOBASE + CNTRL_GRP_1, 0x00);
    outb(IOBASE + CNTRL_GRP_2, 0x00);
    outb(IOBASE + CNTRL_GRP_3, 0x00);
    outb(IOBASE + CNTRL_GRP_4, 0x00);

    printf("Testing DIO outputs all ports all bits 5 seconds HIGH 5 seconds low repeated\n");
    while (1) {
        sleep(5);
        printf("Outputs High\n");
        outb(IOBASE + PA_GRP_0, 0xff);
        outb(IOBASE + PB_GRP_0, 0xff);
        outb(IOBASE + PC_GRP_0, 0xff);
        outb(IOBASE + PA_GRP_1, 0xff);
        outb(IOBASE + PB_GRP_1, 0xff);
        outb(IOBASE + PC_GRP_1, 0xff);
        outb(IOBASE + PA_GRP_2, 0xff);
        outb(IOBASE + PB_GRP_2, 0xff);
        outb(IOBASE + PC_GRP_2, 0xff);
        outb(IOBASE + PA_GRP_3, 0xff);
        outb(IOBASE + PB_GRP_3, 0xff);
        outb(IOBASE + PC_GRP_3, 0xff);
        outb(IOBASE + PA_GRP_4, 0xff);
        outb(IOBASE + PB_GRP_4, 0xff);
        outb(IOBASE + PC_GRP_4, 0xff);
        sleep(5);
        printf("Outputs Low\n");
        outb(IOBASE + PA_GRP_0, 0x00);
        outb(IOBASE + PB_GRP_0, 0x00);
        outb(IOBASE + PC_GRP_0, 0x00);
        outb(IOBASE + PA_GRP_1, 0x00);
        outb(IOBASE + PB_GRP_1, 0x00);
        outb(IOBASE + PC_GRP_1, 0x00);
        outb(IOBASE + PA_GRP_2, 0x00);
        outb(IOBASE + PB_GRP_2, 0x00);
        outb(IOBASE + PC_GRP_2, 0x00);
        outb(IOBASE + PA_GRP_3, 0x00);
        outb(IOBASE + PB_GRP_3, 0x00);
        outb(IOBASE + PC_GRP_3, 0x00);
        outb(IOBASE + PA_GRP_4, 0x00);
        outb(IOBASE + PB_GRP_4, 0x00);
        outb(IOBASE + PC_GRP_4, 0x00);
    } */

}




