#include <stdlib.h>

#include <avr/io.h>
#include <util/delay.h>

#include "radio.h"
#include "serial.h"

#define SPI_SS              PB2
#define SPI_SCK             PB5
#define SPI_MISO            PB4
#define SPI_MOSI            PB3
#define SPI_DDR            DDRB
#define SPI_PORT          PORTB

#define OP_MODE_SLEEP      0x00
#define OP_MODE_STDBY      0x04
#define OP_MODE_FS         0x08
#define OP_MODE_RX         0x10
#define OP_MODE_TX         0x0C

#define MAX_POWER_LEVEL      23

static inline void set_power_level(uint8_t pwl)
{
	uint8_t pa_mask;

	if (pwl < 16) {
		pwl += 16;
		pa_mask = 0x40;
	} else {
		pwl += pwl < 20 ? 10 : 8;
		pa_mask = 0x40 | 0x20;
	}

	write_reg(0x5A, 0x5D);
	write_reg(0x5C, 0x7C);
	write_reg(0x11, (pa_mask | pwl));
}

static inline set_mode(uint8_t mode)
{
	write_reg(0x01, ((read_reg(0x01) & 0xE3) | mode));

	switch (mode) {
	case OP_MODE_TX:
		write_reg(0x5A, 0x5D);
		write_reg(0x5C, 0x7C);
		break;
	case OP_MODE_RX:
	}
}

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
	
	do {
		write_reg(0x2F, 0xAA);
	} while (read_reg(0x2F) != 0xAA);

	do {
		write_reg(0x2F, 0x55);
	} while (read_reg(0x2F) != 0x55);

	write_reg(0x01, 0x04);

	write_reg(0x02, 0x00);

	write_reg(0x03, 0x02);
	write_reg(0x04, 0x40);

	write_reg(0x05, 0x03);
	write_reg(0x06, 0x33);

	// carrier frequency: 433MHz
	write_reg(0x07, 0x6C);
	write_reg(0x08, 0x40);
	write_reg(0x09, 0x00);

	write_reg(0x19, (0x40 | 0x00 | 0x02));

	// DIO mapping
	write_reg(0x25, 0x40);
	write_reg(0x26, 0x07);
	
	write_reg(0x28, 0x10);
	
	write_reg(0x29, 220);
	
	// sync value
	write_reg(0x2E, (0x80 | 0x08));
	write_reg(0x2F, 0x2D);
	write_reg(0x30, cfg->network_id);

	// packet format and FIFO thresholds
	write_reg(0x37, (0x80 | 0x10));
	write_reg(0x38, 66);
	write_reg(0x3C, (0x80 | 0x0F));
	write_reg(0x3D, 0x10);

	write_reg(0x6F, 0x30);

	// disable encryption
	write_reg(0x3D, ((read_reg(0x3D) & 0xFE) | 0x00));

	// enable high power amps
	write_reg(0x13, 0x0F);
	set_power_level(MAX_POWER_LEVEL);

	write_reg(0x01, ((read_reg(0x01) & 0xE3) | 0x04));
	while (!(read_reg(0x27) & 0x80))
		;
}
