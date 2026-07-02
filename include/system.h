#ifndef SYSTEM_H
#define SYSTEM_H

uint8_t sysInit();

uint8_t GPIOInit();

uint8_t SPIInit();

uint8_t SysTickInit();

void sysDelay(volatile uint32_t count);

#endif 
