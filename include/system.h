#ifndef SYSTEM_H
#define SYSTEM_H

void sysInit();

void GPIOInit();

void SysTickInit();

void sysDelay(volatile uint32_t count);

#endif 
