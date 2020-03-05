
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "external/APCI/apcilib/apcilib.h"
#include "include/pci_dio_120.h"

int pci_dio_120_fd;

void init_pci_dio_120() {

    pci_dio_120_fd = open("/dev/apci/pci_dio_120_0", O_RDWR);

    if (pci_dio_120_fd < 0)
    {
        perror("Error in opening PCI-DIO-120");
        printf("Device file could not be opened. "
               "Please ensure the iogen driver module is loaded and the program us run with 'sudo'.\n");
        exit(0);
    }
}


void write_pci_dio_120(int reg, uint8_t data) {
    int status;

    status = apci_write8(pci_dio_120_fd, 1, 2, reg, data);

    if(status < 0) {
        perror("Error in writing PCI-DIO-120");
    }
}

uint8_t read_pci_dio_120(int reg) {
    int status;
    uint8_t data;

    status = apci_read8(pci_dio_120_fd, 1, 2, reg, &data);

    if(status < 0) {
        perror("Error in reading PCI-DIO-120");
    }

    return data;
}
