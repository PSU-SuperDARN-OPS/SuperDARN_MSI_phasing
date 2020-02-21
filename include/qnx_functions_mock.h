//
// Created by bgklug on 2/21/20.
//

#ifndef PHASINGCALIBRATION_QNX_FUNCTIONS_MOCK_H
#define PHASINGCALIBRATION_QNX_FUNCTIONS_MOCK_H
#include <stdint.h>

uint8_t in8( uintptr_t port );
void out8( uintptr_t port,
           uint8_t val );


#endif //PHASINGCALIBRATION_QNX_FUNCTIONS_MOCK_H
