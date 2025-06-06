#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "nrfm.h"

#define SPI_SS            PB2
#define SPI_SCK           PB5
#define SPI_MISO          PB4
#define SPI_MOSI          PB3
#define SPI_DDR           DDRB
#define SPI_PORT          PORTB

#define NRF_CE            PC1  
#define NRF_CE_DDR        DDRC
#define NRF_CE_PORT       PORTC

#define NOP               0xFF
#define R_REGISTER        0x1F
#define W_REGISTER        0x20

#define PWR_UP            1
#define PRIM_RX           0

#define MODCHG_DELAY_MS   5

#define LEN(a)     (sizeof(a) / sizeof(a[0]))

const char *bittab[16] = {
	[ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
	[ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
	[ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
	[12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

static inline uint8_t read_reg(uint8_t reg)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = reg & R_REGISTER;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = NOP;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);
	return SPDR;
}

static inline void write_reg(uint8_t reg, uint8_t val)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = (reg & 0x1F) | W_REGISTER;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = val;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);
}

static inline void read_reg_bulk(uint8_t reg, uint8_t *data, uint8_t n)
{
	uint8_t i;

	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = reg & R_REGISTER;
	while (!(SPSR & (1 << SPIF)))
		;
	for (i = 0; i < n; i++) {
		SPDR = NOP;
		while (!(SPSR & (1 << SPIF)))
			;
		data[i] = SPDR;
	}
	SPI_PORT |= (1 << SPI_SS);
}

static inline void setaddr(uint8_t reg, const uint8_t addr[ADDRLEN])
{
	int i;

	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = (reg & 0x1F) | W_REGISTER;
	while (!(SPSR & (1 << SPIF)))
		;
	for (i = ADDRLEN - 1; i >= 0; i--) {
		SPDR = addr[i];
		while (!(SPSR & (1 << SPIF)))
			;
	}
	SPI_PORT |= (1 << SPI_SS);
}

static inline void reset_irqs(void)
{
	uint8_t rv;

	rv = read_reg(0x07);
	if (rv != 0b00001110)
		write_reg(0x07, 0b01111110);
}

static inline void tx_mode(void)
{
	uint8_t rv;

	rv = read_reg(0x00);
	if ((rv & 0x03) != 0x02) {
		rv |= (1 << PWR_UP);
		rv &= ~(1 << PRIM_RX);
		write_reg(0x00, rv);
		_delay_ms(MODCHG_DELAY_MS);
	}
}

static inline void rx_mode(void)
{
	uint8_t rv;

	rv = read_reg(0x00);
	if ((rv & 0x03) != 0x02) {
		rv |= (1 << PWR_UP);
		rv |= (1 << PRIM_RX);
		write_reg(0x00, rv);
		_delay_ms(MODCHG_DELAY_MS);
	}
}

static inline void enable_chip(void)
{
	NRF_CE_PORT |= (1 << NRF_CE);
	_delay_us(130);
}

static inline void disable_chip(void)
{
	NRF_CE_PORT &= ~(1 << NRF_CE);
}

static inline void flush_tx(void)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = 0b11100001;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);

	reset_irqs();
}

void radio_flush_rx(void)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = 0b11100010;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);

	reset_irqs();
}

static inline uint8_t rx_pdlen(void)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = 0b01100000;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = NOP;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);
	return SPDR;
}

void radio_init(const uint8_t rxaddr[ADDRLEN])
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);

	NRF_CE_DDR |= (1 << NRF_CE);
	NRF_CE_PORT &= ~(1 << NRF_CE);

	_delay_ms(110); /* power on reset delay */

	write_reg(0x00, 0b00111100);  /* use 2-byte CRC, enable only the rx interrupt  */
	write_reg(0x01, 0b00111111);  /* enable auto ack on all pipes */
	write_reg(0x02, 0b00000011);  /* enable rx address on pipes 0 and 1 */
	write_reg(0x03, 0b00000001);  /* set address width to 3 bytes */
	write_reg(0x04, 0b11111111);  /* 4000uS retransmission delay, 15 tries */
	write_reg(0x05, 0b01110011);  /* use 2.515GHz channel */
	write_reg(0x06, 0b00001110);  /* set data rate to 1Mbps */
	write_reg(0x1D, 0b00000100);  /* enable dynamic payload length */
	write_reg(0x1C, 0b00111111);  /* enable dynamic payload length for all pipes */

	reset_irqs();
	setaddr(0x0B, rxaddr);  /* pipe 1 for rx, pipe 0 for auto-ack */
}

void radio_listen(void)
{
	disable_chip();
	rx_mode();
	enable_chip();
}

void radio_pwr_dwn(void)
{
	uint8_t rv;

	disable_chip();
	rv = read_reg(0x00);
	rv &= ~(1 << PWR_UP);
	write_reg(0x00, rv);
}

uint8_t radio_sendto(const uint8_t addr[ADDRLEN], const char *msg, uint8_t n)
{
	int i, imax;
	uint8_t cfg, rv, maxrt, txds;

	disable_chip();

	cfg = read_reg(0x00);

	tx_mode();
	flush_tx();

	setaddr(0x10, addr);
	setaddr(0x0A, addr);

	imax = n < MAXPDLEN ? n : MAXPDLEN;

	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = 0b10100000;
	while (!(SPSR & (1 << SPIF)))
		;
	for (i = 0; i < imax; i++) {
		SPDR = msg[i];
		while (!(SPSR & (1 << SPIF)))
		;
	}
	SPI_PORT |= (1 << SPI_SS);

	enable_chip();
	_delay_us(12);
	disable_chip();

	txds = 0, maxrt = 0;
	do {
		rv = read_reg(0x07);	
		txds = rv & (1 << 5);
		maxrt = rv & (1 << 4);
	} while (txds == 0 && maxrt == 0);

	if (maxrt)
		flush_tx();

	// restore config, typically rx mode
	write_reg(0x00, cfg);
	enable_chip();
	return txds;
}

uint8_t radio_recv(char *buf, uint8_t n)
{
	int readlen, pdlen, readmax;

	pdlen = 0;
	disable_chip();

	pdlen = rx_pdlen();	
	if (pdlen == 0) {
		radio_flush_rx();
		return 0;
	}
	
	if (pdlen > MAXPDLEN) {
		radio_flush_rx();
		return 0;
	}

	readmax = n < pdlen ? n : pdlen;

	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = 0b01100001;
	while (!(SPSR & (1 << SPIF)))
		;
	for (readlen = 0; readlen < readmax; readlen++) {
		SPDR = NOP;
		while (!(SPSR & (1 << SPIF)))
			;
		buf[readlen] = SPDR;
	}
	SPI_PORT |= (1 << SPI_SS);

	radio_flush_rx();
	enable_chip();
	return readlen;
}
