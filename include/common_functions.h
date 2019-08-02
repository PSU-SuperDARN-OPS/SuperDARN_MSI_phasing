//
// Created by bgklug on 8/2/19.
//

#ifndef PHASINGCALIBRATION_COMMON_FUNCTIONS_H
#define PHASINGCALIBRATION_COMMON_FUNCTIONS_H
#include <stdint.h>

#define SWITCHES 0
#define ATTEN    1
#define READ     0
#define WRITE    1
#define ON       1
#define OFF      0

/*-SET WRITE ENABLE BIT-------------------------------------------------------*/
int32_t set_WE(int32_t base,int32_t onoff,int32_t radar);
/*-SET READ/WRITE BIT-------------------------------------------------------*/
int32_t set_RW(int32_t base,int32_t rw,int32_t radar);

/*-SET SWITCHED/ATTEN BIT-------------------------------------------------------*/
int32_t set_SA(int32_t base,int32_t sa,int32_t radar);


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
int32_t verify_data_new(uint32_t base, int32_t card, int32_t code, int32_t data,int32_t radar,int32_t print);

/*-WRITE_CODE--------------------------------------------------------*/
int32_t write_data_new(uint32_t base, int32_t card, int32_t code, int32_t data,int32_t radar,int32_t print);

/*-WRITE_CODE--------------------------------------------------------*/
int32_t write_data_old(uint32_t base, int32_t card, int32_t code, int32_t data,int32_t radar,int32_t print);

/*-VERITY_CODE--------------------------------------------------------*/

int verify_data_old(unsigned int base, int card, int code, int data,int radar,int print);


/*-READ_DATA---------------------------------------------------------*/
int32_t read_data(uint32_t base,int32_t radar);







#endif //PHASINGCALIBRATION_COMMON_FUNCTIONS_H
