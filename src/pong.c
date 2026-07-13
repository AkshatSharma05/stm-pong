#include "pong.h"

Ball ball;
Paddle p_l;
Paddle p_r;

Paddle *active_paddle = &p_r;

uint8_t delay = 0;

typedef enum {
    GAME_STATE_START = 0,
    GAME_STATE_PLAY
} GameState;

static void game_reset(void);
static void draw_start_screen(void);

void game_loop(){
    GameState state = GAME_STATE_START;

    game_reset();

    while(1){
        oled_clear();

        if (state == GAME_STATE_START)
        {
            draw_start_screen();

            if (pb5_pressed())
            {
                while (pb5_pressed()) {
                }

                game_reset();
                state = GAME_STATE_PLAY;
            }
        }
        else
        {
            game_update();

            if (delay) {
                sysDelay(1000);
                delay = 0;
            }
        }

        oled_update();
    }
}

static void game_reset(void)
{
    p_l.x = 6;
    p_l.y = 26;

    p_r.x = 118;
    p_r.y = 26;

    ball.x = SCREEN_WIDTH/2 - BALL_RADIUS/2;
    ball.y = SCREEN_HEIGHT/2 - BALL_RADIUS/2;

    ball.dx = 2;
    ball.dy = 2;
}

static void draw_start_screen(void)
{
    draw_glyph(49, 28, glyph_P);
    draw_glyph(55, 28, glyph_O);
    draw_glyph(62, 28, glyph_N);
    draw_glyph(69, 28, glyph_G);
}

void game_update(void)
{   
    paddle_update();

    ball_update();

    // oled_draw_line(10,10, 120,40);

    // Draw ball
    oled_fill_circle(ball.x, ball.y, BALL_RADIUS);

    // // Draw paddles
    oled_draw_rect_filled(p_l.x, p_l.y, PADDLE_WIDTH, PADDLE_HEIGHT);
    oled_draw_rect_filled(p_r.x, p_r.y, PADDLE_WIDTH, PADDLE_HEIGHT);
}

void ball_update(){
    int16_t paddle_center = active_paddle->y + PADDLE_HEIGHT / 2;
    int16_t ball_center   = ball.y;

    int16_t error = ball_center - paddle_center;

    // Move ball
    ball.x += ball.dx;
    ball.y += ball.dy;

    // Bounce off left/right walls
    if (ball.x <= BALL_RADIUS)
    {
        ball.x = SCREEN_WIDTH/2 - BALL_RADIUS/2;
        ball.y = SCREEN_HEIGHT/2 - BALL_RADIUS/2;

        ball.dx = 2;
        ball.dy = 2;

        delay = 1;
    }

    if (ball.x >= SCREEN_WIDTH - BALL_RADIUS - 1)
    {
        ball.x = SCREEN_WIDTH/2 - BALL_RADIUS/2;
        ball.y = SCREEN_HEIGHT/2 - BALL_RADIUS/2;

        ball.dx = -2;
        ball.dy = -2;

        delay = 1;
    }

    // Bounce off top/bottom walls
    if (ball.y <= BALL_RADIUS)
    {
        ball.y = BALL_RADIUS;
        ball.dy *= -1;
    }

    if (ball.y >= SCREEN_HEIGHT - BALL_RADIUS - 1)
    {
        ball.y = SCREEN_HEIGHT - BALL_RADIUS - 1;
        ball.dy *= -1;
    }

    if (ball.dx < 0 &&
    ball.x - BALL_RADIUS <= p_l.x + PADDLE_WIDTH &&
    ball.x + BALL_RADIUS >= p_l.x &&
    ball.y + BALL_RADIUS >= p_l.y &&
    ball.y - BALL_RADIUS <= p_l.y + PADDLE_HEIGHT)
    {
        ball.dx *= -1;
        ball.x = p_l.x + PADDLE_WIDTH + BALL_RADIUS;

        if (error < -4)
            ball.dy = -2;
        else if (error < -2)
            ball.dy = -1;
        else if (error < 2)
            ball.dy = 0;
        else if (error < 4)
            ball.dy = 1;
        else
            ball.dy = 2;
    }

    if (ball.dx > 0 &&
    ball.x + BALL_RADIUS >= p_r.x &&
    ball.x - BALL_RADIUS <= p_r.x + PADDLE_WIDTH &&
    ball.y + BALL_RADIUS >= p_r.y &&
    ball.y - BALL_RADIUS <= p_r.y + PADDLE_HEIGHT)
    {
        ball.dx *= -1;
        ball.x = p_r.x - BALL_RADIUS;

        if (error < -4)
            ball.dy = -2;
        else if (error < -2)
            ball.dy = -1;
        else if (error < 2)
            ball.dy = 0;
        else if (error < 4)
            ball.dy = 1;
        else
            ball.dy = 2;
    }
}

void paddle_update(void)
{
    if(ball.dx > 0) {
        active_paddle = &p_r;
    }else{ active_paddle = & p_l; }

    // Move active paddle with encoder
    int32_t step = enc_get_delta();
    if (step == 0)
        return;

    int16_t next_y = active_paddle->y + step * 4;

    if (next_y < 0)
        next_y = 0;
    else if (next_y > (SCREEN_HEIGHT - 20))
        next_y = SCREEN_HEIGHT - 20;

    active_paddle->y = next_y;
}
