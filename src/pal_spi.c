#include "regs.h"
#include "pal_spi.h"

uint8_t pal_spi_init( void )
{
    RCC_APB2ENR |= RCC_APB2ENR_SPI1EN;

    /*
    Using BSRR -> atomic operations

    BSRR is a 32-bit write-only register.

    Bits 0-15: writing a 1 sets the corresponding output bit.
    Bits 16-31: writing a 1 clears the corresponding output bit.
    */

    // Set CS RES DC -> HIGH until used
    GPIOA_BSRR = ((1U << 2) | (1U << 3) | (1U << 4));

    // in the datasheet: full-duplex (BIDIMODE=0 and RXONLY=0)
    SPI_CR1 =
        (1U << 9) |   // SSM -> Software slave management
        (1U << 8) |   // SSI -> Internal slave select
        (SPI_BR_DIV2) |
        (1U << 6) |   // SPE -> SPI Enable
        (1U << 2);    // MSTR
        //CPOL and CPHA = 0 

    SPI_CR2 = 0x0;

    /*
        The TXE flag (Tx buffer empty) is set when the data are transferred from the Tx buffer to the
        shift register. It indicates that the internal Tx buffer is ready to be loaded with the next data.
        An interrupt can be generated if the TXEIE bit in the SPI_CR2 register is set. Clearing the
        TXE bit is performed by writing to the SPI_DR register.

        Note:
        The software must ensure that the TXE flag is set to 1 before attempting to write to the Tx
        buffer. Otherwise, it overwrites the data previously written to the Tx buffer.


        The RXNE flag (Rx buffer not empty) is set on the last sampling clock edge, when the data
        are transferred from the shift register to the Rx buffer. It indicates that data are ready to be
        read from the SPI_DR register. An interrupt can be generated if the RXNEIE bit in the
        SPI_CR2 register is set. Clearing the RXNE bit is performed by reading the SPI_DR
        register.
        For some configurations, the BSY flag can be used during the last data transfer to wait until
        the completion of the transfer.
    */

    //Prior to changing the CPOL/CPHA bits the SPI must be disabled by resetting the SPE bit.
    // CPOL : Clock Polarity -> defines if ACTIVE LOW (CPOL = 0) or ACTIVE HIGH (CPOL = 1)
    // CPHA : Clock Phase -> defines if data is latched on the first or second half of the clock period.

    return 0;
}


//writes to the SPI_DR Register -> redirected to the internal shift registers
uint8_t pal_spi_send( uint8_t data ) {
    //TX should only be done when TX buffer is empty 
    while (!(SPI_SR & SPI_SR_TXE)) { /*IDLE*/ }
    
    SPI_DR = data;

    //WHY BSY -> when data is sent TX buffer becomes empty -> but shift register is still sending bits, hence we need to wait till the last bit has been shifted.
    // while (SPI_SR & SPI_SR_BSY) { /*IDLE while bus has not become idle*/ } OR
    while(!(SPI_SR & SPI_SR_RXNE)) { /*IDLE while RX buffer is empty -> when a bit is received, return it*/ }
    
    return SPI_DR; //same register is used for both RX and TX, internally there is separate hardware for both, but the window is same
}
