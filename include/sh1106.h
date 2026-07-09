#ifndef SH1106_H
#define SH1106_H

#include <stdint.h>
#include "pal_io.h" //contains helper functions for IO
#include "regs.h"
#include "pal_spi.h"
#include "system.h"

//OLED Functions
void oled_init( );

void oled_clear( );

void oled_command( uint8_t cmd );
void oled_data( uint8_t cmd );

void oled_set_cursor( uint8_t page, uint8_t column);

void oled_fill( uint8_t pattern );

void oled_reset( );

#endif