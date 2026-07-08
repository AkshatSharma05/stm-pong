#include "regs.h"
#include "pal_spi.h"

void sysDelay(volatile uint32_t count)
{
    while (count--);
}

uint8_t pal_gpio_init(){

    RCC_APB2ENR |= (RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPAEN);

    //GPIO C
    GPIOC_CRH &= ~(0xFU << 20);
    GPIOC_CRH |=  (0x3U << 20);

    //GPIO B
    GPIOB_CRL &= ~(0xFU << 20);
    GPIOB_CRL |=  (0x8U << 20);

    GPIOB_ODR |= (1U << 5);

    //GPIO A -> 2 , 3 , 4 -> Output Push Pull

    // PA2
    GPIOA_CRL &= ~(0xFU << 8);
    GPIOA_CRL |=  (0x3U << 8);

    // PA3
    GPIOA_CRL &= ~(0xFU << 12);
    GPIOA_CRL |=  (0x3U << 12);

    // PA4
    GPIOA_CRL &= ~(0xFU << 16);
    GPIOA_CRL |=  (0x3U << 16);

    // PA5 , 7 -> Alternate Function Push Pull
    GPIOA_CRL &= ~(0xFU << 20);
    GPIOA_CRL |=  (0xBU << 20);

    GPIOA_CRL &= ~(0xFU << 28);
    GPIOA_CRL |=  (0xBU << 28);

    return 0;
}

uint8_t SysTickInit(void){
    /*
        If CLKSOURCE = AHB
        The SysTick input clock becomes:
        8,000,000 Hz

        Since 1Hz = 1cycle/second
        then
        8,000,000Hz=8,000,000cycles/second
        Now suppose you want a delay of 1 ms.
        Convert milliseconds to seconds:
        1ms=0.001s
        Now apply the formula:

        N=f×t
        N=8,000,000×0.001
        N=8000 cycles
        So the timer must count 8000 clock ticks.

        SYSTICK_CTRL -> ENABLE = 1 , TICKINT = 1, CLKSRC = 1 (8 MHz)
    */
    SYSTICK_CTRL = SYSTICK_ENABLE | SYSTICK_TICKINT | SYSTICK_CLKSOURCE;

    /*A write of any value clears the field to 0, and also clears the COUNTFLAG bit in the
    STK_CTRL register to 0.*/
    SYSTICK_VAL  = 0;

    SYSTICK_LOAD = 0x1F3F;

    return 0;

}

uint8_t sysInit(void){
    SysTickInit();
    pal_gpio_init();
    pal_spi_init();

    return 0;
}
