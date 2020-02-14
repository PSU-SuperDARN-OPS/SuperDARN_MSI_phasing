//
// Created by bgklug on 8/13/19.
//

#ifndef PHASINGCALIBRATION_OPEN_PLX9050_H
#define PHASINGCALIBRATION_OPEN_PLX9050_H

#define VENDOR_ID 0x494f
//#define DEVICE_ID 0x0e60
#define DEVICE_ID 0x0c78

unsigned int 	BASEIO;
//int		vPLX9050_INTCSR = 0x4c;

int _open_PLX9052(int *pci_handle, unsigned int *mmap_io_ptr, int *interrupt_line, int print);

#endif //PHASINGCALIBRATION_OPEN_PLX9050_H
