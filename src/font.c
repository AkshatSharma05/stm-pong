#include "font.h"

const uint8_t glyph_A[5] =
{
    0x7E,
    0x11,
    0x11,
    0x11,
    0x7E
};

const uint8_t glyph_B[5] =
{
    0x7F,
    0x49,
    0x49,
    0x49,
    0x36
};

const uint8_t glyph_0[5] =
{
    0x3E,
    0x51,
    0x49,
    0x45,
    0x3E
};

const uint8_t glyph_1[5] =
{
    0x00,
    0x42,
    0x7F,
    0x40,
    0x00
};

const uint8_t glyph_2[5] =
{
    0x42,
    0x61,
    0x51,
    0x49,
    0x46
};

const uint8_t glyph_3[5] =
{
    0x22,
    0x41,
    0x49,
    0x49,
    0x36
};

const uint8_t glyph_4[5] = { 0x10, 0x18, 0x14, 0x7F, 0x10 };

const uint8_t glyph_5[5] = 
{ 
    0x4F, 
    0x49, 
    0x49, 
    0x49, 
    0x79 
};

const uint8_t glyph_6[5] =
{
    0x06,
    0x29,
    0x49,
    0x49,
    0x3E
};

const uint8_t glyph_7[5] =
{
    0x40,
    0x47,
    0x48,
    0x50,
    0x60
};

const uint8_t glyph_8[5] =
{
    0x36,
    0x49,
    0x49,
    0x49,
    0x36
};

const uint8_t glyph_9[5] =
{
    0x3E,
    0x49,
    0x49,
    0x49,
    0x30
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

const uint8_t glyph_R[5] =
{
    0x7F,
    0x09,
    0x19,
    0x29,
    0x46
};

const uint8_t glyph_E[5] =
{
    0x7F,
    0x49,
    0x49,
    0x49,
    0x41
};

const uint8_t glyph_S[5] =
{
    0x46,
    0x49,
    0x49,
    0x49,
    0x31
};

const uint8_t glyph_T[5] =
{
    0x01,
    0x01,
    0x7F,
    0x01,
    0x01
};

const uint8_t glyph_W[5] =
{
    0x7F,
    0x20,
    0x18,
    0x20,
    0x7F
};

const uint8_t glyph_I[5] =
{
    0x41,
    0x7F,
    0x41,
    0x00,
    0x00
};

// const uint8_t glyph_5[5] =
// {

//     0x7F, 
//     0x49, 
//     0x49, 
//     0x49,
//     0x39
// };

void draw_glyph( uint8_t x, uint8_t y, const uint8_t glyph[5]){
    draw_glyph_scaled(x, y, glyph, 1);
}

void draw_glyph_scaled(uint8_t x, uint8_t y, const uint8_t glyph[5], uint8_t scale)
{
    for(uint8_t col = 0; col < 5; col++){
        uint8_t column = glyph[col];
        for(uint8_t row = 0; row < 7; row ++){
            if (!(column & (1 << row)))
                continue;

            if (scale == 1)
            {
                oled_set_pixel(x + col, y + row);
                continue;
            }

            for (uint8_t dx = 0; dx < scale; dx++)
            {
                for (uint8_t dy = 0; dy < scale; dy++)
                {
                    oled_set_pixel(x + (col * scale) + dx, y + (row * scale) + dy);
                }
            }
        }
    }
}
