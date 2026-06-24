/*
 * include/regs.h
 *
 * Hand-written register map. Only registers this project currently touches
 * are defined here — new peripherals get added as each milestone needs them.
 * All addresses and offsets come directly from the STM32F103 reference
 * manual (RM0008), memory map chapter and individual peripheral chapters.
 *
 * Every register is declared as:
 *   (*(volatile uint32_t *)(BASE + OFFSET))
 *
 * volatile: tells the compiler this memory location can change or have
 *   side effects independent of the program — disables read/write
 *   optimizations that would be correct for ordinary RAM but wrong for
 *   hardware registers.
 *
 * uint32_t: STM32F1 peripheral registers are all 32-bit wide. Reading or
 *   writing a narrower type (uint8_t, uint16_t) at these addresses produces
 *   undefined or incorrect behavior on this bus — always use 32-bit
 *   accesses, masking bits yourself when you only care about a subset.
 */

#ifndef REGS_H
#define REGS_H

#include <stdint.h>


#define RCC_BASE        0x40021000UL

#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))

/* Bit 4 of APB2ENR = IOPCEN: I/O port C clock enable.
 * Writing 1 here starts the clock signal into the GPIOC peripheral block.
 * Writing 0 stops it (and loses any configuration you had set). */
#define RCC_APB2ENR_IOPCEN  (1U << 4)

/* -------------------------------------------------------------------------
 * GPIOC — General Purpose I/O port C
 * Base: 0x40011000  (RM0008, memory map, "APB2 peripherals")
 * -------------------------------------------------------------------------
 * STM32F1 GPIO is configured differently from later STM32 families:
 * instead of separate MODE and OTYPE registers, each pin gets a 4-bit
 * field in CRL (pins 0-7) or CRH (pins 8-15) that encodes both the
 * direction and the output type together. The field is:
 *   [1:0] MODE — 00: input, 01: output 10MHz, 10: output 2MHz, 11: output 50MHz
 *   [3:2] CNF  — meaning depends on MODE:
 *                  input:  00=analog, 01=floating, 10=pull-up/pull-down
 *                  output: 00=push-pull, 01=open-drain, 10=AF push-pull, 11=AF open-drain
 *
 * PC13 is the onboard LED (active-low: drive low = LED on).
 * PC13 is pin 13 -> lives in CRH, field at bits [23:20].
 * We want: MODE=11 (output 50MHz), CNF=00 (push-pull) -> field value = 0b0011 = 0x3
 * Speed selection (50MHz here) only affects output slew rate, not correctness —
 * it's the standard default used in virtually every reference example.
 */
#define GPIOC_BASE      0x40011000UL

/* CRH: port configuration register high. Offset 0x04.
 * Configures pins 8-15, four bits per pin. */
#define GPIOC_CRH       (*(volatile uint32_t *)(GPIOC_BASE + 0x04))

/* ODR: output data register. Offset 0x0C.
 * Bit N reflects the driven logic level on pin N when that pin is
 * configured as output. Write 0 to a bit -> pin goes low. Write 1 -> high.
 * PC13 = bit 13. Active-low LED: clear bit 13 to turn on, set to turn off. */
#define GPIOC_ODR       (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))

#endif /* REGS_H */
