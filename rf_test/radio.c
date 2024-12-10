#include <stdlib.h>

#include <avr/io.h>
#include <util/delay.h>

#include "radio.h"
#include "serial.h"

#define SPI_SS       PB2
#define SPI_SCK      PB5
#define SPI_MISO     PB4
#define SPI_MOSI     PB3
#define SPI_DDR      DDRB
#define SPI_PORT     PORTB

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

	write_reg(0x01, 0x04);	
	while ((read_reg(0x27) & 0x80))
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

	// todo: high power settings?

	write_reg(0x01, 0x0C);
	while (!(read_reg(0x28) & 0x08))
		;

	write_reg(0x01, 0x04);
	while ((read_reg(0x27) & 0x80))
		;
}

uint8_t radio_recv(char *buf, uint8_t n)
{
	uint8_t read_len;

	read_len = 0;

	write_reg(0x01, 0x04);
	while ((read_reg(0x27) & 0x80))
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
	return read_len;
}

void radio_listen(void)
{
	write_reg(0x01, (read_reg(0x01) & 0xE3) | 0x10);
	while ((read_reg(0x27) & 0x80))
		;

	// todo: go to low power mode
}

void radio_init(const struct radio_cfg *cfg)
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);

	// LNA, AFC and RXBW settings
	write_reg(0x18, 0x88);
	write_reg(0x19, 0x55);
	write_reg(0x1A, 0x8B);

	// DIO mappings: IRQ on DIO0
	write_reg(0x25, 0x40);
	write_reg(0x26, 0x07);

	// RSSI threshold
	write_reg(0x29, 0xE4);

	// sync config
	write_reg(0x2E, 0x80);
	write_reg(0x2F, cfg->netid);

	// packet config
	write_reg(0x37, 0x10);
	write_reg(0x38, cfg->payload_len);
	write_reg(0x39, cfg->nodeid);

	// fifo config
	write_reg(0x3C, 0x8F);

	// DAGC config
	write_reg(0x6F, 0x30);
}
