#include <stdint.h>
#include "regs.h"
#include "system.h"

int main(void)
{
    sysInit();

    /*
     *   PC13 low  (bit 13 = 0) → LED on   (current flows through LED)
     *   PC13 high (bit 13 = 1) → LED off  (no voltage differential)
     *
     * We use separate &= and |= operations (read-modify-write) on ODR
     * rather than writing the whole register — same reason as CRH above,
     * don't clobber other pins on port C that might be configured later.
     */
    for (;;) {
        // PC13 is low enabled
        GPIOC_ODR &= ~(1U << 13);  /* pull PC13 low  → LED on  */
        sysDelay(100000);
        GPIOC_ODR |=  (1U << 13);  /* pull PC13 high → LED off */
        sysDelay(100000);
    }

    return 0;
}
