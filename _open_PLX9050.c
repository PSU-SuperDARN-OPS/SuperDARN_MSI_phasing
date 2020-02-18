#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#ifdef __QNX__
  #include <hw/pci.h>
  #include <hw/inout.h>
  #include <sys/neutrino.h>
  #include <sys/mman.h>
#endif

#include "include/_open_PLX9050.h"



int _open_PLX9052(int *pci_handle, unsigned int *mmap_io_ptr, int *interrupt_line, int print){
#ifdef __QNX__
    volatile unsigned char *BASE0, *BASE1;
    unsigned flags, lastbus, version, hardware, bus, device;
    int temp;
    unsigned pci_index = 0;
    struct _pci_config_regs pci_reg;

    /* CONNECT TO PCI SERVER */
    *pci_handle = pci_attach(flags);
    if (*pci_handle == -1) {
        perror("Cannot attach to PCI system");
        return -1;
    }

    /* CHECK FOR PCI BUS */
    temp = pci_present(&lastbus, &version, &hardware);
    if (temp != PCI_SUCCESS) {
        perror("Cannot find PCI BIOS");
        return -1;
    }

    /* FIND DEVICE */
    temp = pci_find_device(DEVICE_ID, VENDOR_ID, pci_index, &bus, &device);
    if (temp != PCI_SUCCESS) {
        perror("Cannot find GC314-PCI/FS Digital Receiver Card");
        return -1;
    }

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

    return 1;
#else
    if(print) {
        printf("Debug on linux, pci_handle: %d, mmap: %d, interrupt: %d", *pci_handle, *mmap_io_ptr, *interrupt_line);
    }
    return 1;
#endif
}
