#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#ifdef __QNX__
  #include <hw/pci.h>
  #include <hw/inout.h>
  #include <sys/neutrino.h>
  #include <sys/mman.h>
#else
#include "include/qnx_functions_mock.h"
#endif

#include "include/registers.h"
#include "include/phasing_cards.h"

int32_t set_ports(struct DIO *phasing_matrix) {
    switch(phasing_matrix->radar_number) {
        case 1:
            phasing_matrix->port.A0 = PA_GRP_0;
            phasing_matrix->port.A1 = PA_GRP_1;
            phasing_matrix->port.B0 = PB_GRP_0;
            phasing_matrix->port.B1 = PB_GRP_1;
            phasing_matrix->port.C0 = PC_GRP_0;
            phasing_matrix->port.C1 = PC_GRP_1;
            phasing_matrix->port.cntrl0 = CNTRL_GRP_0;
            phasing_matrix->port.cntrl1 = CNTRL_GRP_1;
            break;
        case 2:
            phasing_matrix->port.A0 = PA_GRP_2;
            phasing_matrix->port.A1 = PA_GRP_3;
            phasing_matrix->port.B0 = PB_GRP_2;
            phasing_matrix->port.B1 = PB_GRP_3;
            phasing_matrix->port.C0 = PC_GRP_2;
            phasing_matrix->port.C1 = PC_GRP_3;
            phasing_matrix->port.cntrl0 = CNTRL_GRP_2;
            phasing_matrix->port.cntrl1 = CNTRL_GRP_3;
            break;
        case 3:
            phasing_matrix->port.A0 = PA_GRP_4;
            phasing_matrix->port.A1 = PA_GRP_3;
            phasing_matrix->port.B0 = PB_GRP_4;
            phasing_matrix->port.B1 = PB_GRP_3;
            phasing_matrix->port.C0 = PC_GRP_4;
            phasing_matrix->port.C1 = PC_GRP_3;
            phasing_matrix->port.cntrl0 = CNTRL_GRP_4;
            phasing_matrix->port.cntrl1 = CNTRL_GRP_3;
            break;
        default:
            return -1;
    }
    return 0;
}

void disable_write(struct DIO const *phasing_matrix) {
    uint8_t temp;

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.C0);
    out8(phasing_matrix->base_address + phasing_matrix->port.C0, temp & WRITE_DISABLE_MASK);
}

void enable_write(struct DIO const *phasing_matrix) {
    uint8_t temp;

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.C0);
    out8(phasing_matrix->base_address + phasing_matrix->port.C0, temp | WRITE_ENABLE_MASK);
}

void select_read(struct DIO const *phasing_matrix) {
    uint8_t temp;

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.C0);
    out8(phasing_matrix->base_address + phasing_matrix->port.C0, temp & READ_SELECT_MASK);
}

void select_write(struct DIO const *phasing_matrix) {
    uint8_t temp;

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.C0);
    out8(phasing_matrix->base_address + phasing_matrix->port.C0, temp | WRITE_SELECT_MASK);
}

void select_phase(struct DIO const *phasing_matrix) {
    uint8_t temp;

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.C0);
    out8(phasing_matrix->base_address + phasing_matrix->port.C0, temp & PHASE_SELECT_MASK);

}

void select_attenuator(struct DIO const *phasing_matrix) {
    uint8_t temp;

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.C0);
    out8(phasing_matrix->base_address + phasing_matrix->port.C0, temp | ATTENUATOR_SELECT_MASK);
}

int32_t reverse_bits(int32_t data) {

    int32_t temp = 0;

    temp = temp + ((data & 1) << 12);
    temp = temp + ((data & 2) << 10);
    temp = temp + ((data & 4) << 8);
    temp = temp + ((data & 8) << 6);
    temp = temp + ((data & 16) << 4);
    temp = temp + ((data & 32) << 2);
    temp = temp + ((data & 64) << 0);
    temp = temp + ((data & 128) >> 2);
    temp = temp + ((data & 256) >> 4);
    temp = temp + ((data & 512) >> 6);
    temp = temp + ((data & 1024) >> 8);
    temp = temp + ((data & 2048) >> 10);
    temp = temp + ((data & 4096) >> 12);

    return temp;
}

float get_delay(int32_t code) {

    int32_t i;
    float delay;
    float delaylist[] = {0.25, 0.45f, 0.8f, 1.5, 2.75, 5, 8, 15, 25, 45, 80, 140, 250};

    delay = 0;
    for (i = 0; i < 13; i++) {
        delay += ((code & (int32_t) pow(2, i)) >> i) * delaylist[i];
    }
    return delay;
}

int32_t beam_code(struct DIO const *phasing_matrix, int32_t code) {
    /* the beam code is 13 bits, pAD0 thru pAD12.  This code
       uses bits 0-7 of CH0, PortA, and bits 0-4 of CH0, PortB
       to output the beam code. Note: The beam code is an address
       of the EEPROMs in the phasing cards.  This code is broadcast
       to ALL phasing cards.  If you are witing the EEPROM, then this
       be the beam code you are writing
    */
    uint16_t temp;

    // check if beam code is reasonable
    if ((code > 8192) | (code < 0)) {
        fprintf(stderr, "INVALID BEAM CODE - must be between 0 and 8192\n");
        fflush(stderr);
        return -1;
    }
    // bit reverse the code
    code = reverse_bits(code);
    // set CH0, Port A to lowest 8 bits of beam code and output on PortA
    temp = code & 0xff;

    out8(phasing_matrix->base_address + phasing_matrix->port.A0, temp);


    // set CH0, Port B to upper 5 bits of beam code and output on PortB
    temp = code & 0x1f00;
    temp = temp >> 8;

    out8(phasing_matrix->base_address + phasing_matrix->port.B0, temp);

    // verify that proper beam code was sent out

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.B0);
    temp = (temp & 0x1f) << 8;
    temp = temp + in8(phasing_matrix->base_address + phasing_matrix->port.A0);


    if (temp != code) {
        fprintf(stderr, "BEAM CODE OUTPUT ERROR - requested code not sent\n");
        fflush(stderr);
        return -1;
    }
    return 0;
}

int32_t select_card(struct DIO const *phasing_matrix, int32_t address) {
    /* This code selects a card to address.  This can be used for
       writing data to the EEPROM, or to verify the output of the
       EEPROM. There are 20 cards in the phasing matrix, addresses
       0-19.  A card is addressed when this address corresponds to
       the switches on the phasing card.  Card address 31 is reserved for
       programming purposes.
    */
    int32_t temp;

    // check if card address is reasonable
    if ((address > 31) | (address < 0)) {
        fprintf(stderr, "INVALID CARD ADDRESS - must be between 0 and 32\n");
        fflush(stderr);
        return -1;
    }
    // shift address left 1 bit (write enable is the lowest bit)
    address = address << 1;
    // mask out bits not used for addressing the cards
    address = address & 0x3e;

    // check for other bits in CH0, PortC that may be on
    temp = in8(phasing_matrix->base_address + phasing_matrix->port.C0);
    temp = temp & 0xc1;
    // add other bit of PortC to the address bits
    address = address + temp;
    // output the address and original other bits to PortC
    out8(phasing_matrix->base_address + phasing_matrix->port.C0, address);
    usleep(3000);
    // verify the output
    temp = in8(phasing_matrix->base_address + phasing_matrix->port.C0);

    if (temp != address) {
        fprintf(stderr, "CARD SELECT OUTPUT ERROR - requested code not sent\n");
        fprintf(stderr, " code=%d\n", temp);
        return -1;
    }
    return 0;
}

int32_t write_attenuators(const struct DIO *phasing_matrix, int32_t card, int32_t code, int32_t data) {
    int32_t temp;

    // check that the data to write is valid
    if ((data > 63) | (data < 0)) {
        fprintf(stderr, "INVALID ATTEN DATA TO WRITE - must be between 0 and 63\n");
        return -1;
    }

    data = data ^ 0x3f;
    // select card to write
    temp = select_card(phasing_matrix, card);
    // choose the beam code to write (output appropriate EEPROM address
    temp = beam_code(phasing_matrix, code);
    select_attenuator(phasing_matrix);
    // enable writing
    select_write(phasing_matrix);
    // set CH1, PortA and Port B to output for writing

    out8(phasing_matrix->base_address + phasing_matrix->port.cntrl1, 0x81);

    // bit reverse the data
    data = reverse_bits(data);
    // apply the data to be written to PortA and PortB on CH1
    // set CH1, Port A to lowest 8 bits of data and output on PortA
    temp = data & 0xff;

    out8(phasing_matrix->base_address + phasing_matrix->port.A1, temp);

    // set CH0, Port B to upper 5 bits of data and output on PortB
    temp = data & 0x1f00;
    temp = (temp >> 8);

    out8(phasing_matrix->base_address + phasing_matrix->port.B1, temp);
    out8(phasing_matrix->base_address + phasing_matrix->port.cntrl1, 0x01);

    // toggle write enable bit
    enable_write(phasing_matrix);
    disable_write(phasing_matrix);
    // reset CH1, PortA and PortB to inputs

    out8(phasing_matrix->base_address + phasing_matrix->port.cntrl1, 0x93);
    out8(phasing_matrix->base_address + phasing_matrix->port.cntrl1, 0x13);

    // disable writing
    select_read(phasing_matrix);
    usleep(3000);
    // verify written data
    // read PortA and PortB to see if EEPROM output is same as progammed

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.B1);
    temp = temp & 0x1f;
    temp = temp << 8;
    temp = temp + in8(phasing_matrix->base_address + phasing_matrix->port.A1);
    temp = temp & 0x1f80;


    if (temp != data) {
        printf(" ERROR - ATTEN DATA NOT WRITTEN: data: %x != readback: %x :: Code: %d Card: %d\n", reverse_bits(data),
               reverse_bits(temp), code, card);
        return -1;
    }

    return 0;
}

int32_t verify_attenuators(const struct DIO *phasing_matrix, int32_t card, int32_t code, int32_t data) {
    int32_t temp;

    // check that the data to write is valid
    if ((data > 63) | (data < 0)) {
        fprintf(stderr, "INVALID ATTEN DATA TO VERIFY - must be between 0 and 63\n");
        return -1;
    }
    data = data ^ 0x3f;
    // bit reverse the data
    data = reverse_bits(data);
    // select card to write
    temp = select_card(phasing_matrix, card);
    // choose the beam code to write (output appropriate EEPROM address
    temp = beam_code(phasing_matrix, code);
    select_attenuator(phasing_matrix);
    // disable writing
    select_write(phasing_matrix);
    usleep(10000);
    // verify written data
    // read PortA and PortB to see if EEPROM output is same as progammed

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.B1);
    temp = temp & 0x1f;
    temp = temp << 8;
    temp = temp + in8(phasing_matrix->base_address + phasing_matrix->port.A1);
    temp = temp & 0x1f80;

    if (temp != data) {
        printf(" ERROR - ATTEN DATA NOT VERIFIED: data: %x != readback: %x :: Code: %d Card: %d\n", reverse_bits(data),
               reverse_bits(temp), code, card);
        return -1;
    }
    return 0;
}

int32_t write_data(struct DIO const *phasing_matrix, int32_t card, int32_t code, int32_t data) {
    int32_t temp;

    // check that the data to write is valid
    if ((data > 8192) | (data < 0)) {
        fprintf(stderr, "INVALID DATA TO WRITE - must be between 0 and 8192\n");
        return -1;
    }
    data = data ^ 0x1fff;
    // select card to write
    temp = select_card(phasing_matrix, card);
    // choose the beam code to write (output appropriate EEPROM address
    temp = beam_code(phasing_matrix, code);
    select_phase(phasing_matrix);
    // enable writing
    select_write(phasing_matrix);
    // set CH1, PortA and Port B to output for writing

    out8(phasing_matrix->base_address + phasing_matrix->port.cntrl1, 0x81);

    // bit reverse the data
    data = reverse_bits(data);
    // apply the data to be written to PortA and PortB on CH1
    // set CH1, Port A to lowest 8 bits of data and output on PortA
    temp = data & 0xff;

    out8(phasing_matrix->base_address + phasing_matrix->port.A1, temp);

    // set CH0, Port B to upper 5 bits of data and output on PortB
    temp = data & 0x1f00;
    temp = (temp >> 8);

    out8(phasing_matrix->base_address + phasing_matrix->port.B1, temp);
    out8(phasing_matrix->base_address + phasing_matrix->port.cntrl1, 0x01);


    // toggle write enable bit
    enable_write(phasing_matrix);
    disable_write(phasing_matrix);
    // reset CH1, PortA and PortB to inputs

    out8(phasing_matrix->base_address + phasing_matrix->port.cntrl1, 0x93);
    out8(phasing_matrix->base_address + phasing_matrix->port.cntrl1, 0x13);

    // disable writing
    select_read(phasing_matrix);
    usleep(10000);
    // verify written data
    // read PortA and PortB to see if EEPROM output is same as progammed

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.B1);
    temp = temp & 0x1f;
    temp = temp << 8;
    temp = temp + in8(phasing_matrix->base_address + phasing_matrix->port.A1);
    temp = temp & 0x1fff;

    if (temp != data) {
        printf(" ERROR - New Card DATA NOT WRITTEN: data: %x != readback: %x :: Code: %d Card: %d\n",
               reverse_bits(data), reverse_bits(temp), code, card);
        return -1;
    }

    return 0;
}

int32_t verify_data(const struct DIO *phasing_matrix, int32_t card, int32_t code, int32_t data) {
    int32_t temp;

    // check that the data to write is valid
    if ((data > 8192) | (data < 0)) {
        fprintf(stderr, "INVALID DATA TO VERIFY - must be between 0 and 8192\n");
        return -1;
    }
    data = data ^ 0x1fff;
    // select card to write
    temp = select_card(phasing_matrix, card);
    // choose the beam code to write (output appropriate EEPROM address
    temp = beam_code(phasing_matrix, code);
    select_phase(phasing_matrix);
    // bit reverse the data
    data = reverse_bits(data);

    // reset CH1, PortA and PortB to inputs

    out8(phasing_matrix->base_address + phasing_matrix->port.cntrl1, 0x93);
    out8(phasing_matrix->base_address + phasing_matrix->port.cntrl1, 0x13);

    // disable writing
    select_write(phasing_matrix);
    usleep(10000);
    // verify written data
    // read PortA and PortB to see if EEPROM output is same as progammed

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.B1);
    temp = temp & 0x1f;
    temp = temp << 8;
    temp = temp + in8(phasing_matrix->base_address + phasing_matrix->port.A1);
    temp = temp & 0x1fff;

    if ((temp != data)) {
        printf(" ERROR - New Card DATA NOT VERIFIED: data: %x != readback: %x :: Code: %d Card: %d\n",
               reverse_bits(data), reverse_bits(temp), code, card);
        return -1;
    }
    return 0;
}
