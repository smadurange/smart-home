#include <avr/io.h>

#include "spi.h"

#define SS_PIN   PB2
#define SS_DDR   DDRB
#define SS_PORT  PORTB

void spi_init(void)
{
	SS_DDR |= (1 << SS_PIN);
	SS_PORT |= (1 << SS_PIN);

	DDR_SPI = (1 << DD_MOSI) | (1 << DD_SCK);
	SPCR    = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

uint8_t spi_send(uint8_t data)
{
	SS_PORT |= (1 << SS_PIN);

	SPDR = data;
	while (!(SPSR & (1 << SPIF)))
		;
	data = SPDR;

	SS_PORT &= ~(1 << SS_PIN);

	return data;
}

uint8_t spi_send(uint8_t addr, uint8_t data)
{
	SS_PORT |= (1 << SS_PIN);

	SPDR = addr;
	while (!(SPSR & (1 << SPIF)))
		;

	SPDR = data;
	while (!(SPSR & (1 << SPIF)))
		;

	SS_PORT &= ~(1 << SS_PIN);

	return data;
}

