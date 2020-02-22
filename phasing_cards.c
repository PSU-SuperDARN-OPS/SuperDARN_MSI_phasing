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


void enable_write(const struct DIO *phasing_matrix);

void select_write(struct DIO const *phasing_matrix);

void select_attenuator(struct DIO const *phasing_matrix);

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
    out8(phasing_matrix->base_address + phasing_matrix->port.C0, temp & 0xbf);
}

void select_write(struct DIO const *phasing_matrix) {
    uint8_t temp;

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.C0);
    out8(phasing_matrix->base_address + phasing_matrix->port.C0, temp | 0x40);
}

void select_phase(struct DIO const *phasing_matrix) {
    uint8_t temp;

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.C0);
    out8(phasing_matrix->base_address + phasing_matrix->port.C0, temp & 0x7f);

}

void select_attenuator(struct DIO const *phasing_matrix) {
    uint8_t temp;

    temp = in8(phasing_matrix->base_address + phasing_matrix->port.C0);
    out8(phasing_matrix->base_address + phasing_matrix->port.C0, temp | 0x80);
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

int32_t beam_code(uint32_t base, int32_t code, int32_t radar) {
    /* the beam code is 13 bits, pAD0 thru pAD12.  This code
       uses bits 0-7 of CH0, PortA, and bits 0-4 of CH0, PortB
       to output the beam code. Note: The beam code is an address
       of the EEPROMs in the phasing cards.  This code is broadcast
       to ALL phasing cards.  If you are witing the EEPROM, then this
       be the beam code you are writing
    */

    int32_t temp;
    struct Port port;

    temp = set_ports(&port);
    if(temp < 0) {
        printf("Invalid radar number");
    }

#ifndef __QNX__
    printf("Debug linux, base: %d", base);
#endif

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
#ifdef __QNX__
    out8(base + port.A0, temp);
#else
    printf("Set output on Port A to %d", temp);
#endif
    // set CH0, Port B to upper 5 bits of beam code and output on PortB
    temp = code & 0x1f00;
    temp = temp >> 8;
#ifdef __QNX__
    out8(base + port.B0, temp);
#else
    printf("Set output on Port B to %d", temp);
#endif
    // verify that proper beam code was sent out
#ifdef __QNX__
    temp = in8(base + port.B0);
    temp = (temp & 0x1f) << 8;
    temp = temp + in8(base + port.A0);
#else
    temp = code;
#endif

    if (temp == code) {
        return 0;
    } else {
        fprintf(stderr, "BEAM CODE OUTPUT ERROR - requested code not sent\n");
        fflush(stderr);
        return -1;
    }


}

int32_t select_card(uint32_t base, int32_t address, int32_t radar) {

    /* This code selects a card to address.  This can be used for
       writing data to the EEPROM, or to verify the output of the
       EEPROM. There are 20 cards in the phasing matrix, addresses
       0-19.  A card is addressed when this address corresponds to
       the switches on the phasing card.  Card address 31 is reserved for
       programming purposes.
    */
    int32_t temp;
    struct Port port;

    struct 	timespec nsleep;
    nsleep.tv_sec=0;
    nsleep.tv_nsec=5000;


    temp = set_ports(&port);
    if(temp < 0) {
        printf("Invalid radar number");
    }

#ifndef __QNX__
    printf("Debug linux, base: %d", base);
#endif

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
#ifdef __QNX__
    // check for other bits in CH0, PortC that may be on
    temp = in8(base + port.C0);
    temp = temp & 0xc1;
    // add other bit of PortC to the address bits
    address = address + temp;
    // output the address and original other bits to PortC
    out8(base + port.C0, address);
    nanosleep(&nsleep, NULL); //TODO: see if this is necessary or if usleep would work
    // verify the output
    temp = in8(base + port.C0);
#else
    temp = address;
#endif
    if (temp != address) {
        fprintf(stderr, "CARD SELECT OUTPUT ERROR - requested code not sent\n");
        fprintf(stderr, " code=%d\n", temp);
        return -1;
    }
    return 0;
}

int32_t write_attenuators(uint32_t base, int32_t card, int32_t code, int32_t data, int32_t radar) {
    int32_t temp;
    struct Port port;

    temp = set_ports(&port);
    if(temp < 0) {
        printf("Invalid radar number");
    }

    // check that the data to write is valid
    if ((data > 63) | (data < 0)) {
        fprintf(stderr, "INVALID ATTEN DATA TO WRITE - must be between 0 and 63\n");
        return -1;
    }

    data = data ^ 0x3f;
    // select card to write
    temp = select_card(base, card, radar);
    // choose the beam code to write (output appropriate EEPROM address
    temp = beam_code(base, code, radar);
    select_attenuator(radar);
    // enable writing
    select_write(radar);
    // set CH1, PortA and Port B to output for writing
#ifdef __QNX__
    out8(base + port.cntrl1, 0x81);
#endif
    // bit reverse the data
    data = reverse_bits(data);
    // apply the data to be written to PortA and PortB on CH1
    // set CH1, Port A to lowest 8 bits of data and output on PortA
    temp = data & 0xff;
#ifdef __QNX__
    out8(base + port.A1, temp);
#endif
    // set CH0, Port B to upper 5 bits of data and output on PortB
    temp = data & 0x1f00;
    temp = (temp >> 8);
#ifdef __QNX__
    out8(base + port.B1, temp);
    out8(base + port.cntrl1, 0x01);
#endif
    // toggle write enable bit
    enable_write(NULL);
    disable_write(NULL);
    // reset CH1, PortA and PortB to inputs
#ifdef __QNX__
    out8(base + port.cntrl1, 0x93);
    out8(base + port.cntrl1, 0x13);
#endif
    // disable writing
    select_read(radar);
    usleep(3000);
    // verify written data
    // read PortA and PortB to see if EEPROM output is same as progammed
#ifdef __QNX__
    temp = in8(base + port.B1);
    temp = temp & 0x1f;
    temp = temp << 8;
    temp = temp + in8(base + port.A1);
    temp = temp & 0x1f80;
#else
    temp = data;
#endif

    if (temp != data) {
        printf(" ERROR - ATTEN DATA NOT WRITTEN: data: %x != readback: %x :: Code: %d Card: %d\n", reverse_bits(data),
               reverse_bits(temp), code, card);
        return -1;
    }

    return 0;
}

int32_t verify_attenuators(uint32_t base, int32_t card, int32_t code, int32_t data, int32_t radar) {
    int32_t temp;
    struct Port port;

    temp = set_ports(&port);
    if(temp < 0) {
        printf("Invalid radar number");
    }

    // check that the data to write is valid
    if ((data > 63) | (data < 0)) {
        fprintf(stderr, "INVALID ATTEN DATA TO VERIFY - must be between 0 and 63\n");
        return -1;
    }
    data = data ^ 0x3f;
    // bit reverse the data
    data = reverse_bits(data);
    // select card to write
    temp = select_card(base, card, radar);
    // choose the beam code to write (output appropriate EEPROM address
    temp = beam_code(base, code, radar);
    select_attenuator(radar);
    // disable writing
    select_write(radar);
    usleep(10000);
    // verify written data
    // read PortA and PortB to see if EEPROM output is same as progammed
#ifdef __QNX__
    temp = in8(base + port.B1);
    temp = temp & 0x1f;
    temp = temp << 8;
    temp = temp + in8(base + port.A1);
    temp = temp & 0x1f80;
#else
    temp = data;
#endif

    if (temp != data) {
        printf(" ERROR - ATTEN DATA NOT VERIFIED: data: %x != readback: %x :: Code: %d Card: %d\n", reverse_bits(data),
               reverse_bits(temp), code, card);
        return -1;
    }
    return 0;
}

int32_t write_data(uint32_t base, int32_t card, int32_t code, int32_t data, int32_t radar, int32_t print) {
    int32_t temp;
    struct Port port;

    temp = set_ports(&port);
    if(temp < 0) {
        printf("Invalid radar number");
    }

    // check that the data to write is valid
    if ((data > 8192) | (data < 0)) {
        fprintf(stderr, "INVALID DATA TO WRITE - must be between 0 and 8192\n");
        return -1;
    }
    data = data ^ 0x1fff;
    if (print) printf("    Code to write is %d\n", data);
    // select card to write
    temp = select_card(base, card, radar);
    // choose the beam code to write (output appropriate EEPROM address
    temp = beam_code(base, code, radar);
    select_phase(NULL);
    // enable writing
    select_write(radar);
    // set CH1, PortA and Port B to output for writing
#ifdef __QNX__
    out8(base + port.cntrl1, 0x81);
#endif
    // bit reverse the data
    data = reverse_bits(data);
    // apply the data to be written to PortA and PortB on CH1
    // set CH1, Port A to lowest 8 bits of data and output on PortA
    temp = data & 0xff;
#ifdef __QNX__
    out8(base + port.A1, temp);
#endif
    // set CH0, Port B to upper 5 bits of data and output on PortB
    temp = data & 0x1f00;
    temp = (temp >> 8);
#ifdef __QNX__
    out8(base + port.B1, temp);
    out8(base + port.cntrl1, 0x01);
#endif

    // toggle write enable bit
    enable_write(NULL);
    disable_write(NULL);
    // reset CH1, PortA and PortB to inputs
#ifdef __QNX__
    out8(base + port.cntrl1, 0x93);
    out8(base + port.cntrl1, 0x13);
#endif
    // disable writing
    select_read(radar);
    usleep(10000);
    // verify written data
    // read PortA and PortB to see if EEPROM output is same as progammed
#ifdef __QNX__
    temp = in8(base + port.B1);
    temp = temp & 0x1f;
    temp = temp << 8;
    temp = temp + in8(base + port.A1);
    temp = temp & 0x1fff;
#else
    temp = data;
#endif

    if (print) {
        printf("    Code read after writing is %d\n", reverse_bits(temp));
    }

    if (temp != data) {
        printf(" ERROR - New Card DATA NOT WRITTEN: data: %x != readback: %x :: Code: %d Card: %d\n",
               reverse_bits(data), reverse_bits(temp), code, card);
        return -1;
    }

    return 0;
}

int32_t verify_data(uint32_t base, int32_t card, int32_t code, int32_t data, int32_t radar, int32_t print) {
    int32_t temp;
    struct Port port;

    temp = set_ports(&port);
    if(temp < 0) {
        printf("Invalid radar number");
    }

    // check that the data to write is valid
    if ((data > 8192) | (data < 0)) {
        fprintf(stderr, "INVALID DATA TO VERIFY - must be between 0 and 8192\n");
        return -1;
    }
    data = data ^ 0x1fff;
    // select card to write
    temp = select_card(base, card, radar);
    // choose the beam code to write (output appropriate EEPROM address
    temp = beam_code(base, code, radar);
    select_phase(NULL);
    // bit reverse the data
    data = reverse_bits(data);
    if (print) printf("    Code to write is %d\n", data);
    // reset CH1, PortA and PortB to inputs
#ifdef __QNX__
    out8(base + port.cntrl1, 0x93);
    out8(base + port.cntrl1, 0x13);
#endif
    // disable writing
    select_write(radar);
    usleep(10000);
    // verify written data
    // read PortA and PortB to see if EEPROM output is same as progammed
#ifdef __QNX__
    temp = in8(base + port.B1);
    temp = temp & 0x1f;
    temp = temp << 8;
    temp = temp + in8(base + port.A1);
    temp = temp & 0x1fff;
#else
    temp = data;
#endif

    if (print) {
        printf("    data expected: %d data read: %d\n", data, temp);
    }

    if ((temp != data)) {
        printf(" ERROR - New Card DATA NOT VERIFIED: data: %x != readback: %x :: Code: %d Card: %d\n",
               reverse_bits(data), reverse_bits(temp), code, card);
        return -1;
    }
    return 0;
}
