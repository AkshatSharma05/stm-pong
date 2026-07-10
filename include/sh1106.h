#ifndef SH1106_H
#define SH1106_H

#include <stdint.h>
#include "pal_io.h" //contains helper functions for IO
#include "regs.h"
#include "pal_spi.h"
#include "system.h"

//DEFINITIONS
extern uint8_t oled_framebuffer[ 128 * 8 ]; // 1 page = 1 byte tall and 128 columns -> 8 * 128 bytes

//OLED Functions
void oled_init( );

void oled_clear( );

void oled_command( uint8_t cmd );
void oled_data( uint8_t cmd );

void oled_set_cursor( uint8_t page, uint8_t column);

void oled_fill( uint8_t pattern );
void oled_clear_pixel(uint8_t x, uint8_t y);
void oled_set_pixel(uint8_t x, uint8_t y);

void oled_clear( );

void oled_reset( );

#endif