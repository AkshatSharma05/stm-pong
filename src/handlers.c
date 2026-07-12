#include <stdint.h>
#include "handlers.h"
#include "pal_io.h"

volatile uint32_t sysTicks = 0;

void SysTick_Handler(void)
{
    sysTicks++;
    enc_sample(); //sample encoder steps each systick interrupt
}
