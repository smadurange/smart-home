#include <stdlib.h>
#include <string.h>

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

static inline uint8_t read_reg(uint8_t reg)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = reg & 0x7F;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = 0;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);

	return SPDR;
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

static inline void radio_send(const char *data, uint8_t n)
{
	uint8_t i;

	// STDBY + ListenAbort mode
	write_reg(0x01, 0x04);
	while ((read_reg(0x27) >> 7) != 1)
		;

	// queue data to FIFO register
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = 0x00 | 0x80;
	while (!(SPSR & (1 << SPIF)))
		;
	for (i = 0; i < n; i++) {
		SPDR = data[i];
		while (!(SPSR & (1 << SPIF)))
			;
	}
	SPI_PORT |= (1 << SPI_SS);

	// initiate transmission by switching to TX mode
	write_reg(0x01, 0x0C);	
}

static inline void radio_init(void)
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);
}

int main(void)
{
	const char *s = "hello";

	serial_init();
	radio_init();

	for (;;) {
		radio_send(s, strlen(s));
		serial_write_line("sent data");
		_delay_ms(2000);
	}

	return 0;
}

