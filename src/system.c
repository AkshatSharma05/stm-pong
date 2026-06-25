#include "regs.h"

void sysDelay(volatile uint32_t count)
{
    while (count--);
}

void GPIOInit(){
    /*
     * Step 1: enable GPIOC (IO PORT C -> IOPC) clock via RCC_APB2ENR bit 4 (IOPCEN).
     *
     * Read-modify-write with |= rather than plain = so we don't disturb
     * any other bits in APB2ENR that might matter (AFIO clock, other GPIO
     * port clocks, etc.). After this line, the GPIOC peripheral block is
     * receiving its bus clock and its registers are live.
     */
    RCC_APB2ENR |= (RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPBEN);

    /*
     * Step 2: configure PC13 as general-purpose push-pull output, 50MHz.
     *
     * PC13 is pin 13, so its 4-bit config field lives in CRH at bits[23:20].
     *
     * First line: clear bits [23:20] to 0000 using a mask.
     *   0xF        = 0b1111 (a 4-bit mask)
     *   0xF << 20  = the mask positioned at bits [23:20]
     *   ~(...)     = inverted: all bits 1 except [23:20] which are 0
     *   &=         = AND with the register: zeroes [23:20], leaves rest alone
     *
     * Second line: write 0x3 (0b0011) into bits [23:20].
     *   CNF[1:0] = 00 → general-purpose push-pull output
     *   MODE[1:0] = 11 → output, max speed 50MHz
     *   0x3 << 20  = value positioned at the right field
     *   |=         = OR into register: sets [23:20], leaves rest alone
     */
    GPIOC_CRH &= ~(0xFU << 20);
    GPIOC_CRH |=  (0x3U << 20);

    GPIOB_CRL &= ~(0xFU << 20);
    GPIOB_CRL |=  (0x8U << 20);

    GPIOB_ODR |= (1U << 5);
}

void SysTickInit(){
    /*
        If CLKSOURCE = AHB
        The SysTick input clock becomes:
        8,000,000 Hz

        For a 1 ms interrupt:
        8,000,000 / 1000 = 8000 cycles

        RELOAD = N - 1 gives:
        RELOAD = 7999

        SYSTICK_CTRL -> ENABLE = 1 , TICKINT = 1, CLKSRC = 1 (8 MHz)
    */
    SYSTICK_CTRL = SYSTICK_ENABLE | SYSTICK_TICKINT | SYSTICK_CLKSOURCE;

    /*A write of any value clears the field to 0, and also clears the COUNTFLAG bit in the
    STK_CTRL register to 0.*/
    SYSTICK_VAL  = 0;

    SYSTICK_LOAD = 0x1F3F;

}

void sysInit(){
    GPIOInit();
    SysTickInit();

}