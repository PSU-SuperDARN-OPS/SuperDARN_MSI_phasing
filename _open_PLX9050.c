
//#define __KERNEL__
//#define CONFIG_PCI

#include <stdio.h>
#include <stdint.h>
#include <linux/types.h>
#include <fcntl.h>
#include <stdlib.h>


#include "include/_open_PLX9050.h"

#include "external/APCI/apcilib/apcilib.h"

const uint32_t BASE_ADDRESS = 0x4080;
const uint32_t BUS_LENGTH = 120 * 8;


int _open_PLX9052(int *mmap_io_ptr) {
    int temp;

    temp = open("/dev/apci/pci_dio_1290_0", O_RDWR);

    if (temp < 0)
    {
        printf("Device file could not be opened. Please ensure the iogen driver module is loaded.\n");
        exit(0);
    }

    *mmap_io_ptr = temp;

    return temp;
}


