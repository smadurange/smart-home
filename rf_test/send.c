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

#define RX_PIN         PB0
#define RX_DDR         DDRB
#define RX_PORT        PORTB
#define RX_PCIE        PCIE0
#define RX_PCINT       PCINT0
#define RX_PCMSK       PCMSK0
#define RX_PCINTVEC    PCINT0_vect

#define RX_BUFLEN   32

static char rxbuf[RX_BUFLEN];
static volatile uint8_t rx_ready = 0;

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

	// STDBY + ListenOn mode
	write_reg(0x01, (read_reg(0x01) | 0x40));
}

static inline void radio_recv(char *buf, uint8_t n)
{
	uint8_t i;
}

static inline void radio_init(void)
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);

	RX_DDR &= ~(1 << RX_PIN);
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);
}

int main(void)
{
	const char *s = "hello";

	serial_init();
	radio_init();

	sei();

	for (;;) {
		radio_send(s, strlen(s));
		serial_write_line("sent data");
		_delay_ms(2000);
	}

	return 0;
}

ISR(RX_PCINTVEC)
{
	uint8_t i;
	char buf[RX_BUFLEN];

	if ((read_reg(0x28) & 0x04) != 0)
	{
		for (i = 0; i < RX_BUFLEN; i++) {
		}	
	}
}
