#include "regs.h"
#include "pal_io.h"

static volatile int32_t enc_delta = 0; // changes in interrupt so might be optimized away if not volatile

uint8_t pb5_pressed(void){
    return !(GPIOB_IDR & (1U << 5));
}

uint8_t pb6_low(void){
    return (GPIOB_IDR & (1U << 6));
}

uint8_t pb7_low(void){
    return (GPIOB_IDR & (1U << 7));
}

void oled_cs_low (){
    GPIOA_ODR &= ~(1U << 4); 
}

void oled_cs_high(void){
    GPIOA_ODR |= (1U << 4);
}

void oled_dc_command(void){
    GPIOA_ODR &= ~(1U << 2);
}

void oled_dc_data(void){
    GPIOA_ODR |= (1U << 2);
}

void oled_reset_low(void){
    GPIOA_ODR &= ~(1U << 3);
}

void oled_reset_high(void){
    GPIOA_ODR |= (1U << 3);
}

//ENCODER HELPERS

void enc_sample(void)
{
    static uint8_t prev_clk = 0;

    uint8_t clk = pb7_low();
    uint8_t dt  = pb6_low();

    if (!prev_clk && clk) { // rising edge
        if (dt)
            enc_delta++;
        else
            enc_delta--;
    }

    prev_clk = clk;
}

int32_t enc_get_delta(void){
    int32_t delta = enc_delta;
    enc_delta = 0;
    return delta;
}