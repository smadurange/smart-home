#include <avr/io.h>

#include "rfm.h"

#define SS_PIN  PB2
#define SS_DDR  DDRB
#define SS_PORT PORTB

#define SCK_PIN  PB5
#define MOSI_PIN PB3
#define MISO_PIN PB4

#define SPI_DDR DDRB

#define READ_MASK  0x7F
#define WRITE_MASK 0x80

static inline void spi_init(void)
{
	SS_DDR |= (1 << SS_PIN);
	SS_PORT |= (1 << SS_PIN);

	SPI_DDR = (1 << MOSI_PIN) | (1 << SCK_PIN);
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

static inline uint8_t spi_read(uint8_t addr)
{
	SS_PORT |= (1 << SS_PIN);

	SPDR = addr;
	while (!(SPSR & (1 << SPIF)))
		;

	addr = SPDR;

	SS_PORT &= ~(1 << SS_PIN);

	return addr;
}

static inline void spi_write(uint8_t addr, uint8_t data)
{
	SS_PORT |= (1 << SS_PIN);

	SPDR = addr;
	while (!(SPSR & (1 << SPIF)))
		;

	SPDR = data;
	while (!(SPSR & (1 << SPIF)))
		;

	SS_PORT &= ~(1 << SS_PIN);
}

static inline void send_cmd(uint8_t addr, uint8_t val)
{
	uint8_t ra, wa;

	ra = addr | READ_MASK;
	wa = addr | WRITE_MASK; 

	while (spi_read(ra) != val)
		spi_write(wa, val);
}

void rfm_init(uint8_t addr)
{
	spi_init();

	// mode: standby + packet
	send_cmd(0x01, 0x44);

	// rx interrupt on DPIO0
	send_cmd(0x25, 0x40);
	send_cmd(0x26, 0x07);

	// packet format: 8 bits + whitening + crc + addr filtering
	send_cmd(0x37, 0x52);
	send_cmd(0x38, 0x08);
	send_cmd(0x38, addr);

	// disable encryption
	send_cmd(0x3D, 0x02);
}

void rfm_send(uint8_t addr, uint8_t data)
{
}
