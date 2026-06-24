#include <stdint.h>
#include "handlers.h"

volatile uint32_t sysTicks = 0;

void SysTick_Handler(void)
{
    sysTicks++;
}