#include <avr/io.h>
#include <util/delay.h>

#include "radio.h"

#define SPI_SS          PB2
#define SPI_SCK         PB5
#define SPI_MISO        PB4
#define SPI_MOSI        PB3
#define SPI_DDR         DDRB
#define SPI_PORT        PORTB

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

void radio_send(const char *data, uint8_t n)
{
	uint8_t i;

	// STDBY + ListenAbort
	write_reg(0x01, 0x04);
	while ((read_reg(0x27) >> 7) != 1)
		;

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

	write_reg(0x01, 0x0C);
	while (!read_reg(0x28))
		;

	write_reg(0x01, 0x04);
	while ((read_reg(0x27) >> 7) != 1)
		;

	// todo: do a more reliable check
	_delay_ms(10);

	// ListenOn
	write_reg(0x01, (read_reg(0x01) | 0x40));
}

uint8_t radio_recv(char *buf, uint8_t n)
{
	uint8_t read_len;

	read_len = 0;

	if ((read_reg(0x28) & 0x04))
	{
		write_reg(0x01, 0x04);
		while ((read_reg(0x27) >> 7) != 1)
			;

		SPI_PORT &= ~(1 << SPI_SS);
		SPDR = 0x00 | 0x7F;
		while (!(SPSR & (1 << SPIF)))
			;

		while (read_len < n) {
			SPDR = 0;		
			while (!(SPSR & (1 << SPIF)))
				;
			buf[read_len++] = SPDR;
		}	
		SPI_PORT |= (1 << SPI_SS);
	}
	return read_len;
}

void radio_init(struct radio_cfg *cfg)
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);

	// standby
	write_reg(0x01, 0x04);
	while ((read_reg(0x27) >> 7) != 1)
		;

	// enable power amplifiers PA1 and PA2
	write_reg(0x13, 0x0F);
	write_reg(0x11, ((read_reg(0x11) & 0x1F) | 0x60));

	// packet format
	if (cfg->payload_len > 0) {
		write_reg(0x37, 0x10);
		write_reg(0x38, cfg->payload_len);
	}
	
	// start listening
	write_reg(0x01, (read_reg(0x01) | 0x40));
}
