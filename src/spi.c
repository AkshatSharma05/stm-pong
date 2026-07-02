#include "regs.h"
#include "spi.h"

uint8_t SPIInit(void)
{
    // SPI1 is part of the APB2 bus, so GPIOInit already covers the clock gate.

    /*
    Using BSRR -> atomic operations

    BSRR is a 32-bit write-only register.

    Bits 0-15: writing a 1 sets the corresponding output bit.
    Bits 16-31: writing a 1 clears the corresponding output bit.
    */

    // Set CS RES DC -> HIGH until used
    GPIOA_BSRR = ((1U << 2) | (1U << 3) | (1U << 4));

    SPI_CR1 =
        (1U << 9) |   // SSM
        (1U << 8) |   // SSI
        (1U << 6) |   // SPE
        (1U << 2);    // MSTR

    SPI_CR2 = 0x0;

    return 0;
}
