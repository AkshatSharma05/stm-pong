#ifndef PAL_IO_H
#define PAL_IO_H

#include <stdint.h>

uint8_t button_pressed( );

//OLED HELPERS

void oled_cs_low(void); //PA4 low
void oled_cs_high(void);//PA4 high

void oled_dc_command(void);//PA2 low
void oled_dc_data(void);//PA2 high

void oled_reset_low(void);//PA3 low
void oled_reset_high(void);//PA3 high


#endif