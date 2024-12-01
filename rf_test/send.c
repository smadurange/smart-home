#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"

#define SPI_SS      PB2
#define SPI_SCK     PB5
#define SPI_MISO    PB4
#define SPI_MOSI    PB3

#define SPI_DDR     DDRB
#define SPI_PORT    PORTB

#define STDBY       0x04
#define LISTEN_ON   0x40

static inline uint8_t read_reg(uint8_t reg)
{
	uint8_t data;

	SPI_PORT &= ~(1 << SPI_SS);

	SPDR = reg | 0x7F;
	while (!(SPSR & (1 << SPIF)))
		;

	SPDR = 0;
	while (!(SPSR & (1 << SPIF)))
		;

	data = SPDR;
	SPI_PORT |= (1 << SPI_SS);

	return data;
}

static inline void write_reg(uint8_t reg, uint8_t val)
{
	SPI_PORT &= ~(1 << SPI_SS);

	SPDR = reg | 0x80;
	while (!(SPSR & (1 << SPIF)))
		;

	SPDR = val;
	while (!(SPSR & (1 << SPIF)))
		;

	SPI_PORT |= (1 << SPI_SS);
}

static inline void radio_init(void)
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);
	
	write_reg(0x01, STDBY);
}

int main(void)
{
	uint8_t val;
	char buf[100];

	serial_init();

	_delay_ms(3000);
	serial_write_line("Initializing radio");
	radio_init();
	serial_write_line("Initialized radio");

	for (;;) {
		val = read_reg(0x01);
		serial_write_line(itoa(val, buf, 16));
		_delay_ms(2000);
	}

	return 0;
}

