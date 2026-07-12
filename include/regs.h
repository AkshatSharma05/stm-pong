#ifndef REGS_H
#define REGS_H

#include <stdint.h>

/////////////////////// RCC /////////////////////////////

#define RCC_BASE        0x40021000UL

#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))

/* Bit 4 of APB2ENR = IOPCEN: I/O port C clock enable.
 * Writing 1 here starts the clock signal into the GPIOC peripheral block.
 * Writing 0 stops it (and loses any configuration you had set). */
#define RCC_APB2ENR_IOPCEN  (1U << 4) 
#define RCC_APB2ENR_IOPBEN  (1U << 3) 
#define RCC_APB2ENR_IOPAEN  (1U << 2) 
#define RCC_APB2ENR_SPI1EN  (1U << 12)

 ////////////////// GPIO ////////////////////////
#define GPIOC_BASE      0x40011000UL
#define GPIOB_BASE      0x40010C00UL
#define GPIOA_BASE      0x40010800UL

/* CRH: port configuration register high. Offset 0x04.
 * Configures pins 8-15, four bits per pin. */
#define GPIOC_CRH       (*(volatile uint32_t *)(GPIOC_BASE + 0x04)) //C13
#define GPIOB_CRL       (*(volatile uint32_t *)(GPIOB_BASE + 0x00)) //B5/6/7
#define GPIOA_CRL       (*(volatile uint32_t *)(GPIOA_BASE + 0x00)) // A2/4/3

/* ODR: output data register. Offset 0x0C.
 * Bit N reflects the driven logic level on pin N when that pin is
 * configured as output. Write 0 to a bit -> pin goes low. Write 1 -> high.
 * PC13 = bit 13. Active-low LED: clear bit 13 to turn on, set to turn off. */
#define GPIOC_ODR       (*(volatile uint32_t *)(GPIOC_BASE + 0x0C)) //output

#define GPIOB_ODR       (*(volatile uint32_t *)(GPIOB_BASE + 0x0C)) // ODR decides Pull-up Pull-Down in Input mode
#define GPIOB_IDR       (*(volatile uint32_t *)(GPIOB_BASE + 0x08)) //input

#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x0C)) 
#define GPIOA_BSRR      (*(volatile uint32_t *)(GPIOA_BASE + 0x10)) 


///////////////// SYSTICK //////////////////////

/* 
    The processor has a 24-bit system timer, SysTick, that counts down from the reload value to
    zero, reloads (wraps to) the value in the LOAD register on the next clock edge, then counts
    down on subsequent clocks.
*/

#define SYSTICK_BASE     0xE000E010UL

//CONFIGURATION REGISTER
#define SYSTICK_CTRL    (*(volatile uint32_t*)(SYSTICK_BASE))
    #define SYSTICK_ENABLE     (1U << 0)
    #define SYSTICK_TICKINT    (1U << 1)
    #define SYSTICK_CLKSOURCE  (1U << 2)

//Contains LOAD value -> Time period
#define SYSTICK_LOAD    (*(volatile uint32_t*)(SYSTICK_BASE + 0x04))

//Contains current value of SysTick Counter -> to be read
#define SYSTICK_VAL     (*(volatile uint32_t*)(SYSTICK_BASE + 0x08))

#define SYSTICK_CALIB   (*(volatile uint32_t*)(SYSTICK_BASE + 0x0C))

//////////////////////////// SPI ///////////////////////////////
#define SPI_BASE        0x40013000UL
#define SPI_CR1         (*(volatile uint32_t*)(SPI_BASE + 0x00))
#define SPI_CR2         (*(volatile uint32_t*)(SPI_BASE + 0x04))
#define SPI_SR          (*(volatile uint32_t*)(SPI_BASE + 0x08))
#define SPI_DR         (*(volatile uint32_t*)(SPI_BASE + 0x0C))

// CR1 : 0000001101000100 -> Try setting SPE -> SPI Enable -> at the end 
//       5432109876543210

// CR2 : 00000000

#endif /* REGS_H */
