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

const uint_fast8_t WRITE_ENABLE_MASK = 1 << 0;
const uint_fast8_t WRITE_DISABLE_MASK = ~(1 << 0);
const uint_fast8_t READ_SELECT_MASK = 0xbf;
const uint_fast8_t WRITE_SELECT_MASK = 0x40;
const uint_fast8_t PHASE_SELECT_MASK = 0x7f;
const uint_fast8_t ATTENUATOR_SELECT_MASK = 0x80;

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
void select_read(struct DIO const *phasing_matrix);
void select_write(struct DIO const *phasing_matrix);

/*-SET SWITCHED/ATTEN BIT-------------------------------------------------------*/
void select_phase(struct DIO const *phasing_matrix);
void select_attenuator(struct DIO const *phasing_matrix);


/*-REVERSE_BITS-------------------------------------------------------*/
int32_t reverse_bits(int32_t data);
/*-GET_DEALY---------------------------------------------------------*/
float get_delay(int32_t code);
/*-BEAM_CODE---------------------------------------------------------*/
int32_t beam_code(struct DIO const *phasing_matrix, int32_t code);
/*-SELECT_CARD------------------------------------------------------*/
int32_t select_card(struct DIO const *phasing_matrix, int32_t address);
int32_t write_attenuators(const struct DIO *phasing_matrix, int32_t card, int32_t code, int32_t data);

int32_t verify_attenuators(const struct DIO *phasing_matrix, int32_t card, int32_t code, int32_t data);

/*-VERIFY_CODE--------------------------------------------------------*/
int32_t verify_data(const struct DIO *phasing_matrix, int32_t card, int32_t code, int32_t data);

/*-WRITE_CODE--------------------------------------------------------*/
int32_t write_data(struct DIO const *phasing_matrix, int32_t card, int32_t code, int32_t data);

/*-READ_DATA---------------------------------------------------------*/
int32_t read_data(uint32_t base,int32_t radar);







#endif //PHASING_CARDS_H