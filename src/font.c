#include "font.h"

const uint8_t glyph_A[5] =
{
    0x7E,
    0x11,
    0x11,
    0x11,
    0x7E
};

const uint8_t glyph_P[5] =
{
    0x7F,
    0x09,
    0x09,
    0x09,
    0x06
};

const uint8_t glyph_O[5] =
{
    0x3E,
    0x41,
    0x41,
    0x41,
    0x3E
};

const uint8_t glyph_N[5] =
{
    0x7F,
    0x08,
    0x10,
    0x20,
    0x7F
};

const uint8_t glyph_G[5] =
{
    0x3E,
    0x41,
    0x49,
    0x49,
    0x7A
};

void draw_glyph( uint8_t x, uint8_t y, const uint8_t glyph[5]){

    for(uint8_t col = 0; col < 5; col++){
        uint8_t column = glyph[col];
        for(uint8_t row = 0; row < 7; row ++){
            if(column & (1 << row)) oled_set_pixel(x + col, y + row);
        }
    }
}
