#ifndef FONT_H
#define FONT_H
#include <stdint.h>
#include "sh1106.h"

extern const uint8_t glyph_A[5];
extern const uint8_t glyph_P[5];
extern const uint8_t glyph_O[5];
extern const uint8_t glyph_N[5];
extern const uint8_t glyph_G[5];


void draw_glyph( uint8_t x, uint8_t y, const uint8_t glyph[5]);

#endif
