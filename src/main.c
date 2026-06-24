/*
 * src/main.c 
 *
 * Clock state at entry: HSI running at 8MHz, no PLL, no config needed.
 * STM32F1 boots on HSI by default — we are already clocked and running
 * before the first line of main() executes, for free.
 */

#include <stdint.h>
#include "regs.h"

/*
 * Busy-wait delay. Spins the core doing nothing for `count` iterations.
 *
 * volatile on the parameter prevents the compiler from optimizing the
 * loop away entirely — without it, an optimizing compiler correctly
 * observes "this loop has no side effects and its result is unused"
 * and deletes it. volatile forces the load/decrement/store to actually
 * happen in hardware.
 *
 * The iteration count is not calibrated to real time yet (no SysTick).
 * At 8MHz HSI, each loop iteration is roughly a handful of cycles.
 * 400000 iterations gives a visually obvious blink period — exact
 * timing doesn't matter for milestone 1, only that it's visible.
 * SysTick-based millisecond delay replaces this in milestone 2.
 */
static void delay(volatile uint32_t count)
{
    while (count--);
}

int main(void)
{
    /*
     * Step 1: enable GPIOC (IO PORT C -> IOPC) clock via RCC_APB2ENR bit 4 (IOPCEN).
     *
     * Read-modify-write with |= rather than plain = so we don't disturb
     * any other bits in APB2ENR that might matter (AFIO clock, other GPIO
     * port clocks, etc.). After this line, the GPIOC peripheral block is
     * receiving its bus clock and its registers are live.
     */
    RCC_APB2ENR |= RCC_APB2ENR_IOPCEN;

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

    /*
     * Step 3: blink forever.
     *
     * PC13 is active-low on the Blue Pill board — the LED's anode is tied
     * to 3.3V and the cathode connects through a resistor to PC13. So:
     *   PC13 low  (bit 13 = 0) → LED on   (current flows through LED)
     *   PC13 high (bit 13 = 1) → LED off  (no voltage differential)
     *
     * We use separate &= and |= operations (read-modify-write) on ODR
     * rather than writing the whole register — same reason as CRH above,
     * don't clobber other pins on port C that might be configured later.
     */
    for (;;) {
        GPIOC_ODR &= ~(1U << 13);  /* pull PC13 low  → LED on  */
        delay(400000);
        GPIOC_ODR |=  (1U << 13);  /* pull PC13 high → LED off */
        delay(400000);
    }

    /* Unreachable. main() never returns in firmware — the for(;;) above
     * runs forever. The linker may warn about "noreturn" if this is
     * absent; the return keeps the C compiler happy about the int
     * return type. Reset_Handler's hang_after_main catches it anyway. */
    return 0;
}
