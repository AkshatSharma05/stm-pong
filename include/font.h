#ifndef FONT_H
#define FONT_H
#include <stdint.h>
#include "sh1106.h"

extern const uint8_t glyph_A[5];
extern const uint8_t glyph_B[5];
extern const uint8_t glyph_0[5];
extern const uint8_t glyph_1[5];
extern const uint8_t glyph_2[5];
extern const uint8_t glyph_3[5];
extern const uint8_t glyph_4[5];
extern const uint8_t glyph_6[5];
extern const uint8_t glyph_7[5];
extern const uint8_t glyph_8[5];
extern const uint8_t glyph_9[5];
extern const uint8_t glyph_P[5];
extern const uint8_t glyph_O[5];
extern const uint8_t glyph_N[5];
extern const uint8_t glyph_G[5];
extern const uint8_t glyph_R[5];
extern const uint8_t glyph_E[5];
extern const uint8_t glyph_S[5];
extern const uint8_t glyph_T[5];
extern const uint8_t glyph_W[5];
extern const uint8_t glyph_I[5];
extern const uint8_t glyph_5[5];


void draw_glyph( uint8_t x, uint8_t y, const uint8_t glyph[5]);
void draw_glyph_scaled(uint8_t x, uint8_t y, const uint8_t glyph[5], uint8_t scale);

#endif
