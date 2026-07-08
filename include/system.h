#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

uint8_t sysInit(void);

uint8_t pal_gpio_init(void);

uint8_t SysTickInit(void);

void sysDelay(volatile uint32_t count);

#endif 
