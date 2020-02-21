//
// Created by bgklug on 8/2/19.
//

#ifndef PHASING_CARDS_H
#define PHASING_CARDS_H
#include <stdint.h>

const uint_fast8_t PHASE_DELAY_SELECT_BIT = 0;
const uint_fast8_t ATTENUATOR_SELECT_BIT = 1;
const uint_fast8_t READ_SELECT_BIT = 0;
const uint_fast8_t WRITE_SELECT_BIT = 1;
const uint_fast8_t WRITE_ENABLE_BIT = 1;
const uint_fast8_t WRITE_DISABLE_BIT = 0;

struct Port{
    uintptr_t A0;
    uintptr_t A1;
    uintptr_t B0;
    uintptr_t B1;
    uintptr_t C0;
    uintptr_t C1;
    uintptr_t cntrl0;
    uintptr_t cntrl1;
};

struct DIO{
    uint_fast8_t radar_number;
    uintptr_t base_address;
    struct Port port;
};

int32_t set_ports(struct DIO *phasing_matrix);

/*-SET WRITE ENABLE BIT-------------------------------------------------------*/
void disable_write(struct DIO const *phasing_matrix);
void enable_write(struct DIO const *phasing_matrix);
/*-SET READ/WRITE BIT-------------------------------------------------------*/
int32_t set_RW(uint32_t base, int32_t rw, int32_t radar);

/*-SET SWITCHED/ATTEN BIT-------------------------------------------------------*/
int32_t set_SA(uint32_t base, int32_t sa, int32_t radar);


/*-REVERSE_BITS-------------------------------------------------------*/
int32_t reverse_bits(int32_t data);
/*-GET_DEALY---------------------------------------------------------*/
float get_delay(int32_t code);
/*-BEAM_CODE---------------------------------------------------------*/
int32_t beam_code(uint32_t base, int32_t code,int32_t radar);
/*-SELECT_CARD------------------------------------------------------*/
int32_t select_card(uint32_t base, int32_t address,int32_t radar);
int32_t write_attenuators(uint32_t base, int32_t card, int32_t code, int32_t data,int32_t radar);

int32_t verify_attenuators(uint32_t base, int32_t card, int32_t code, int32_t data,int32_t radar);

/*-VERIFY_CODE--------------------------------------------------------*/
int32_t verify_data(uint32_t base, int32_t card, int32_t code, int32_t data, int32_t radar, int32_t print);

/*-WRITE_CODE--------------------------------------------------------*/
int32_t write_data(uint32_t base, int32_t card, int32_t code, int32_t data, int32_t radar, int32_t print);

/*-READ_DATA---------------------------------------------------------*/
int32_t read_data(uint32_t base,int32_t radar);







#endif //PHASING_CARDS_H
