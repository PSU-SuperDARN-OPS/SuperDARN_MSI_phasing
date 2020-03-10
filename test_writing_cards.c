//
// Created by bgklug on 3/6/20.
//

#include <stdio.h>
#include <unistd.h>

#include "include/pci_dio_120.h"
#include "include/phasing_cards.h"

void test_card_select(struct DIO * phasing_matrix) {
    int i;

    select_read(phasing_matrix);
    select_phase(phasing_matrix);

    printf("Testing Card Select\n");

    for(i = 0; i < 20; i = i + 1){
        printf("Selecting card: %d\n", i);
        select_card(phasing_matrix, i);
        sleep(2);
    }
}

void test_sa_bit(struct DIO * phasing_matrix) {
    printf("Testing S/A bit\n");
    select_card(phasing_matrix, 0);
    sleep(1);

    printf("Select attenuator (led on)\n");
    select_attenuator(phasing_matrix);
    sleep(2);

    printf("Select phase switch (led off)\n");
    select_phase(phasing_matrix);
    sleep(2);
}

void test_read_write_bit(struct DIO * phasing_matrix) {
    printf("Testing Read/Write bit\n");
    sleep(1);

    printf("Select write (led on)\n");
    select_write(phasing_matrix);
    sleep(2);

    printf("Select read (led off)\n");
    select_read(phasing_matrix);
    sleep(2);
}

int main(){
    struct DIO phasing_matrix;


    init_pci_dio_120();

    phasing_matrix.radar_number = 1;
    set_ports(&phasing_matrix);

    init_phasing_cards(&phasing_matrix);

    test_card_select(&phasing_matrix);

    test_sa_bit(&phasing_matrix);

    test_read_write_bit(&phasing_matrix);

//    printf("Testing writing to phase delay bits");
//    select_card(&phasing_matrix, 0);
//    select_phase(&phasing_matrix);
//    select_write(&phasing_matrix);
//
//    write_pci_dio_120(phasing_matrix.port.cntrl1, 0x81);
//
//    beam_code(&phasing_matrix, 1);


    printf("**** End of Tests ****\n");
    return 0;
}

