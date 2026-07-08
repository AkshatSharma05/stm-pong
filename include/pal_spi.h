#ifndef SPI_H
#define SPI_H

#include <stdint.h>

#define SPI_BR_DIV2      (0U << 3)
#define SPI_BR_DIV4      (1U << 3)
#define SPI_BR_DIV8      (2U << 3)
#define SPI_BR_DIV16     (3U << 3)
#define SPI_BR_DIV32     (4U << 3)
#define SPI_BR_DIV64     (5U << 3)
#define SPI_BR_DIV128    (6U << 3)
#define SPI_BR_DIV256    (7U << 3)

//SPI_SR
#define SPI_SR_TXE       (1U << 1)
#define SPI_SR_BSY       (1U << 7)
#define SPI_SR_RXNE      (1U << 0)

uint8_t pal_spi_init(void);
uint8_t pal_spi_send();

#endif
