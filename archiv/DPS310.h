
#include <inttypes.h>
#include <math.h>
#include <avr/interrupt.h>
#include "grn_TWI.h"

#define DPS310_W 0xee
#define DPS310_R 0xef


int16_t make_signed_16(uint8_t high_byte, uint8_t low_byte);
uint8_t DPS310_read_8(uint8_t reg);
uint8_t DPS310_write(uint8_t reg, uint8_t data);
uint16_t DPS310_init(void);
int32_t DPS310_gettemp(void);
int32_t DPS310_calcTemp(int32_t raw, int32_t m_c0Half,int32_t m_c1);
int32_t DPS310_get_raw_temp(void);
int32_t DPS310_get_komp_temp(void);
int32_t DPS310_get_raw_pres(void);


