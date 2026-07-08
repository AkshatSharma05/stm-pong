#ifndef SH1106_H
#define SH1106_H

#include <stdint.h>
#include "pal_io.h" //contains helper functions for IO
#include "regs.h"

//OLED Functions
uint8_t oled_init( );
uint8_t oled_clear( );
uint8_t oled_command( );
uint8_t oled_data( );
uint8_t oled_reset( );

#endif