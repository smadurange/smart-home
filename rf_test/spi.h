#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void spi_init(void);

uint8_t spi_recv(uint8_t addr);

uint8_t spi_send(uint8_t addr, data);

#endif /* SPI_H */
