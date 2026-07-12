#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

uint8_t sysInit(void);

void game_loop(void);
void game_update(void);

uint8_t pal_gpio_init(void);

uint8_t SysTickInit(void);

void sysDelay(volatile uint32_t ms);

int16_t iabs(int16_t x);

#endif 
