#ifndef PAL_IO_H
#define PAL_IO_H

#include <stdint.h>

uint8_t pb5_pressed(void);
uint8_t pb6_low(void);
uint8_t pb7_low(void);

//OLED HELPERS

void oled_cs_low(void); //PA4 low
void oled_cs_high(void);//PA4 high

void oled_dc_command(void);//PA2 low
void oled_dc_data(void);//PA2 high

void oled_reset_low(void);//PA3 low
void oled_reset_high(void);//PA3 high

//ENCODER HELPERS
void enc_sample(void);
int32_t enc_get_delta(void);

#endif
