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
void oled_clear_pixel( uint8_t x, uint8_t y );
void oled_set_pixel( uint8_t x, uint8_t y );

void oled_draw_hline( uint8_t x, uint8_t y, uint8_t length );
void oled_draw_vline( uint8_t x, uint8_t y, uint8_t length );

void oled_draw_rect( uint8_t x, uint8_t y, uint8_t width, uint8_t height );
void oled_draw_rect_filled( uint8_t x, uint8_t y, uint8_t width, uint8_t height );
void oled_fill_circle(int16_t xc, int16_t yc, int16_t radius);

void oled_clear( );

void oled_reset( );

#endif