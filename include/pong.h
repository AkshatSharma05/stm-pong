#ifndef PONG_H
#define PONG_H
#include "sh1106.h"
#include "font.h"

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define BALL_RADIUS      3
#define PADDLE_WIDTH   4
#define PADDLE_HEIGHT 12

//DECLARATIONS
typedef struct {
    uint8_t x;
    uint8_t y;
    int8_t dx;
    int8_t dy;
} Ball;

typedef struct {
    uint8_t x;
    uint8_t y;
} Paddle;

//FUNCTIONS
void game_loop();

void game_update(void);

void ball_update( );

void paddle_update( );

#endif