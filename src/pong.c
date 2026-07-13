#include "pong.h"

Ball ball;
Paddle p_l;
Paddle p_r;

Paddle *active_paddle = &p_r;

uint8_t delay = 0;
uint8_t p_l_score = 0;
uint8_t p_r_score = 0;

typedef enum {
    GAME_STATE_START = 0,
    GAME_STATE_PLAY,
    GAME_STATE_WIN
} GameState;

static void game_reset(void);
static void draw_start_screen(void);
static void draw_scoreboard(void);
static void draw_win_screen(uint8_t winner);
static void draw_digit(uint8_t x, uint8_t y, uint8_t value);

void game_loop(){
    GameState state = GAME_STATE_START;
    uint8_t winner = 0;

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
        else if (state == GAME_STATE_PLAY)
        {
            game_update();
            draw_scoreboard();

            if (delay) {
                sysDelay(1000);
                delay = 0;
            }

            if (p_l_score >= 5)
            {
                winner = 1;
                state = GAME_STATE_WIN;
                sysDelay(1500);
                game_reset();
            }
            else if (p_r_score >= 5)
            {
                winner = 2;
                state = GAME_STATE_WIN;
                sysDelay(1500);
                game_reset();
            }
        }
        else
        {
            draw_win_screen(winner);
            oled_update();
            sysDelay(2000);
            state = GAME_STATE_START;
            continue;
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

    p_l_score = 0;
    p_r_score = 0;
}

static void draw_start_screen(void)
{
    draw_glyph_scaled(36, 20, glyph_P, 2);
    draw_glyph_scaled(48, 20, glyph_O, 2);
    draw_glyph_scaled(60, 20, glyph_N, 2);
    draw_glyph_scaled(72, 20, glyph_G, 2);

    draw_glyph(36, 44, glyph_P);
    draw_glyph(42, 44, glyph_R);
    draw_glyph(48, 44, glyph_E);
    draw_glyph(54, 44, glyph_S);
    draw_glyph(60, 44, glyph_S);
    draw_glyph(68, 44, glyph_P);
    draw_glyph(74, 44, glyph_B);
    draw_glyph(80, 44, glyph_5);
}

static void draw_digit(uint8_t x, uint8_t y, uint8_t value)
{
    switch (value)
    {
        case 0: draw_glyph(x, y, glyph_0); break;
        case 1: draw_glyph(x, y, glyph_1); break;
        case 2: draw_glyph(x, y, glyph_2); break;
        case 3: draw_glyph(x, y, glyph_3); break;
        case 4: draw_glyph(x, y, glyph_4); break;
        case 5: draw_glyph(x, y, glyph_5); break;
        case 6: draw_glyph(x, y, glyph_6); break;
        case 7: draw_glyph(x, y, glyph_7); break;
        case 8: draw_glyph(x, y, glyph_8); break;
        case 9: draw_glyph(x, y, glyph_9); break;
        default: draw_glyph(x, y, glyph_0); break;
    }
}

static void draw_scoreboard(void)
{
    draw_digit(20, 2, p_l_score);
    draw_digit(108, 2, p_r_score);
}

static void draw_win_screen(uint8_t winner)
{
    draw_glyph(32, 28, glyph_P);
    draw_digit(38, 28, winner);
    draw_glyph(52, 28, glyph_W);
    draw_glyph(58, 28, glyph_I);
    draw_glyph(64, 28, glyph_N);
    draw_glyph(70, 28, glyph_S);
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
        p_r_score++;
        ball.x = SCREEN_WIDTH/2 - BALL_RADIUS/2;
        ball.y = SCREEN_HEIGHT/2 - BALL_RADIUS/2;
        ball.dx = 2;
        ball.dy = 2;
        delay = 1;
        return;
    }

    if (ball.x >= SCREEN_WIDTH - BALL_RADIUS - 1)
    {
        p_l_score++;
        ball.x = SCREEN_WIDTH/2 - BALL_RADIUS/2;
        ball.y = SCREEN_HEIGHT/2 - BALL_RADIUS/2;
        ball.dx = -2;
        ball.dy = -2;
        delay = 1;
        return;
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
