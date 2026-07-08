#include "regs.h"
#include "pal_io.h"

uint8_t button_pressed( ){
    return !(GPIOB_IDR & (1U << 5));
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