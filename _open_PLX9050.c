
//#define __KERNEL__
//#define CONFIG_PCI

#include <stdio.h>
#include <stdint.h>
//#include <linux/types.h>
//#include <linux/pci.h>
#include <sys/io.h>


#include "include/_open_PLX9050.h"

const uint32_t BASE_ADDRESS = 0x4000;
const uint32_t BUS_LENGTH = 128 * 8;


int _open_PLX9052(int *pci_handle, unsigned int *mmap_io_ptr, int *interrupt_line, int print) {
    int temp;
    //struct pci_dev * device= NULL;


    /* FIND DEVICE */
    //device = pci_get_device(DEVICE_ID, VENDOR_ID, device);
    //temp = pci_enable_device(device);
    //if (temp != 0) {
    //    printf("Error enabling PCI device: %d", temp);
    //    return -1;
    //}

    *mmap_io_ptr = BASE_ADDRESS; //pci_resource_start(device, (unsigned long) 0);

    ioperm(*mmap_io_ptr, BUS_LENGTH, 3);

#ifdef __not_defined__

    /* READ THE DEVICE PCI CONFIGURATION */
    temp = pci_read_config32(bus, device, 0, 16, (char *) &pci_reg);
    if (temp != PCI_SUCCESS) {
        perror("Cannot read from configuration space of the GC314 PCI/FS digital receiver card");
        return -1;
    }
    BASEIO = (int) pci_reg.Base_Address_Regs[1] - 1;

    /* ALLOW I/O ACCESS ON THIS THREAD */
    temp = ThreadCtl(_NTO_TCTL_IO, 0);
    if (temp == -1) {
        perror("Unable to attach I/O privileges to thread");
        return -1;
    }

    /* MAP THE IO SPACE TO BE ABLE TO R/W TO PCI IO */
    *mmap_io_ptr = (unsigned int *) mmap_device_io(16, pci_reg.Base_Address_Regs[2]);
    if ((int) mmap_io_ptr == MAP_DEVICE_FAILED) {
        perror("Device I/O mapping failed");
        return -1;
    }

    /* TRY TO READ PCI DEVICE PARAMETERS */
    *interrupt_line = pci_reg.Interrupt_Line;

    /* PRINT PLX9656 PARAMETERS */
    if (print == 1) {
        printf("	PCI DEVICE PARAMETERS:\n");
        printf("	  lastbus=		%d\n", lastbus);
        printf("	  version=		%d\n", version);
        printf("	  hardware=		%d\n", hardware);
        printf("	  bus=			%d\n", bus);
        printf("	  device=		%d\n", device);
        printf("	MEMORY ALLOCATION:\n");
        printf("	  IO Base0=		0x%x\n", pci_reg.Base_Address_Regs[0]);
        printf("	  IO Base1=		0x%x\n", pci_reg.Base_Address_Regs[1]);
        printf("	  IO Base2=		0x%x\n", pci_reg.Base_Address_Regs[2]);
    }

    *mmap_io_ptr = pci_reg.Base_Address_Regs[2] - 1;
#endif
    return 1;
}


