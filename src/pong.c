#include "pong.h"

Ball ball;
Paddle p_l;
Paddle p_r;

Paddle *active_paddle = &p_r;

void game_loop(){
    p_l.x = 6;
    p_l.y = 22;

    p_r.x = 118;
    p_r.y = 22;

    ball.x = 64 - BALL_RADIUS/2;
    ball.y = 32 - BALL_RADIUS/2;

    ball.dx = 2;
    ball.dy = 2;

    while(1){
        oled_clear();
        game_update();
        oled_update();
    }
}

void game_update(void)
{

    paddle_update();

    ball_update();

    // oled_draw_line(10,10, 120,40);

    // Draw ball
    oled_fill_circle(ball.x, ball.y, BALL_RADIUS);

    // // Draw paddles
    oled_draw_rect_filled(p_l.x, p_l.y, 4, 20);
    oled_draw_rect_filled(p_r.x, p_r.y, 4, 20);
}

void ball_update(){
    // Move ball
    ball.x += ball.dx;
    ball.y += ball.dy;

    // Bounce off left/right walls
    if (ball.x <= BALL_RADIUS)
    {
        ball.x = BALL_RADIUS;
        ball.dx *= -1;
    }

    if (ball.x >= SCREEN_WIDTH - BALL_RADIUS - 1)
    {
        ball.x = SCREEN_WIDTH - BALL_RADIUS - 1;
        ball.dx *= -1;
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
}

void paddle_update(void)
{
    static uint8_t prev_btn = 0;

    // Toggle active paddle on button press
    uint8_t btn = pb5_pressed();
    if (btn && !prev_btn)
    {
        if (active_paddle == &p_l)
            active_paddle = &p_r;
        else
            active_paddle = &p_l;
    }
    prev_btn = btn;

    // Move active paddle with encoder
    int32_t step = enc_get_delta();
    if (step == 0)
        return;

    int16_t next_y = active_paddle->y + step * 3;

    if (next_y < 0)
        next_y = 0;
    else if (next_y > (SCREEN_HEIGHT - 20))
        next_y = SCREEN_HEIGHT - 20;

    active_paddle->y = next_y;
}