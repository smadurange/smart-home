#include <avr/io.h>

#include "rfm.h"

#define SS_PIN  PB2
#define SS_DDR  DDRB
#define SS_PORT PORTB

#define SCK_PIN  PB5
#define MOSI_PIN PB3
#define MISO_PIN PB4
#define SPI_DDR  DDRB

// RFM69 op modes
#define RX        0x10
#define TX        0x0C
#define SLEEP     0x00
#define STDBY     0x04
#define LISTEN_ON 0x40

static inline void spi_init(void)
{
	SS_DDR |= (1 << SS_PIN);
	SS_PORT |= (1 << SS_PIN);
	SPI_DDR = (1 << MOSI_PIN) | (1 << SCK_PIN);

	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

static inline uint8_t read_reg(uint8_t reg)
{
	uint8_t data;

	SS_PORT |= (1 << SS_PIN);
	SPDR = addr | 0x7F;
	while (!(SPSR & (1 << SPIF)))
		;
	data = SPDR;
	SS_PORT &= ~(1 << SS_PIN);

	return data;
}

static inline void write_reg(uint8_t reg, uint8_t val)
{
	while (read_reg(reg) != val) {
		SS_PORT |= (1 << SS_PIN);
		SPDR = addr | 0x80;
		while (!(SPSR & (1 << SPIF)))
			;
		SS_PORT &= ~(1 << SS_PIN);
	}
}

static inline void set_mode(uint8_t mode)
{
	write_reg(0x01, mode);
	while (!read_reg(0x27))
		;
}

void rfm_init(void)
{
	spi_init();

	set_mode(STDBY | LISTEN_ON);

	// rx interrupt on DPIO0
	write_reg(0x25, 0x40);
	write_reg(0x26, 0x07);

	// packet format: 8 bits + whitening + crc
	write_reg(0x37, 0x52);
	write_reg(0x38, 0x08);

	// disable encryption
	write_reg(0x3D, 0x02);
}

void rfm_sendto(uint8_t addr, uint8_t *data, uint8_t n)
{
	uint8_t i;

	set_mode(STDBY);

	SS_PORT |= (1 << SS_PIN);

	SPDR = 0x7F;
	while (!(SPSR & (1 << SPIF)))
		;

	SPDR = addr;
	while (!(SPSR & (1 << SPIF)))
		;

	for (i = 0; i < n; i++) {
		SPDR = data[i];
		while (!(SPSR & (1 << SPIF)))
			;
	}	

	SS_PORT &= ~(1 << SS_PIN);

	set_mode(TX);
	
	while (!((read_reg(0x28) >> 3) & 1))
		;
}

uint8_t rfm_recvfrom(uint8_t addr, uint8_t *buf, uint8_t n)
{
	uint8_t i, bytes;
	
	SS_PORT |= (1 << SS_PIN);

	for (i = 0; i < n && ((read_reg(0x28) >> 6) & 1); i++) {
		buf[i] = read_reg(0x00);
		bytes++;
	}

	SS_PORT &= ~(1 << SS_PIN);

	return bytes;
}
