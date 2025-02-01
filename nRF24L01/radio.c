#include <stdlib.h>

#include <avr/io.h>
#include <util/delay.h>

#include "radio.h"
#include "serial.h"

#define SPI_SS     PB2
#define SPI_SCK    PB5
#define SPI_MISO   PB4
#define SPI_MOSI   PB3
#define SPI_DDR    DDRB
#define SPI_PORT   PORTB

#define NRF24L01_CE       PB0
#define NRF24L01_CE_DDR   DDRB
#define NRF24L01_CE_PORT  DDRB

#define NRF24L01_REG_CONFIG           0x00
#define NRF24L01_REG_EN_AA            0x01
#define NRF24L01_REG_EN_RXADDR        0x02
#define NRF24L01_REG_SETUP_AW         0x03
#define NRF24L01_REG_SETUP_RETR       0x04
#define NRF24L01_REG_RF_CH            0x05
#define NRF24L01_REG_RF_SETUP         0x06
#define NRF24L01_REG_STATUS           0x07
#define NRF24L01_REG_OBSERVE_TX       0x08
#define NRF24L01_REG_RPD              0x09
#define NRF24L01_REG_RX_ADDR_P0       0x0A
#define NRF24L01_REG_RX_ADDR_P1       0x0B
#define NRF24L01_REG_RX_ADDR_P2       0x0C
#define NRF24L01_REG_RX_ADDR_P3       0x0D
#define NRF24L01_REG_RX_ADDR_P4       0x0E
#define NRF24L01_REG_RX_ADDR_P5       0x0F
#define NRF24L01_REG_TX_ADDR          0x10
#define NRF24L01_REG_RX_PW_P0         0x11
#define NRF24L01_REG_RX_PW_P1         0x12
#define NRF24L01_REG_RX_PW_P2         0x13
#define NRF24L01_REG_RX_PW_P3         0x14
#define NRF24L01_REG_RX_PW_P4         0x15
#define NRF24L01_REG_RX_PW_P5         0x16
#define NRF24L01_REG_FIFO_STATUS      0x17
#define NRF24L01_REG_DYNPD            0x1C
#define NRF24L01_REG_FEATURE          0x1D

#define NRF24L01_R_REGISTER           0x1F
#define NRF24L01_W_REGISTER           0x20
#define NRF24L01_R_RX_PAYLOAD         0x60
#define NRF24L01_W_TX_PAYLOAD         0xA0
#define NRF24L01_FLUSH_TX             0xE1
#define NRF24L01_FLUSH_RX             0xE2
#define NRF24L01_REUSE_TX_PL          0xE3
#define NRF24L01_R_RX_PL_WID          0x60
#define NRF24L01_W_ACK_PAYLOAD        0xA8
#define NRF24L01_W_TX_PAYLOAD_NOACK   0xB0
#define NRF24L01_NOP                  0xFF

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
	uint8_t i, mode;

	mode = opmode;
	set_mode(RF69_OPMODE_STDBY);

	SPI_PORT &= ~(1 << SPI_SS);

	SPDR = RF69_REG_FIFO | 0x80;
	while (!(SPSR & (1 << SPIF)))
		;

	for (i = 0; i < n; i++) {
		SPDR = data[i];
		while (!(SPSR & (1 << SPIF)))
			;
	}

	SPI_PORT |= (1 << SPI_SS);

	set_mode(RF69_OPMODE_TX);
	while (!(read_reg(RF69_REG_IRQFLAGS2) & 0x08))
		;

	set_mode(mode);
}

uint8_t radio_recv(char *buf, uint8_t n)
{
	uint8_t read_len, mode;

	read_len = 0;
	mode = opmode;

	set_mode(RF69_OPMODE_STDBY);

	SPI_PORT &= ~(1 << SPI_SS);

	SPDR = RF69_REG_FIFO | 0x7F;
	while (!(SPSR & (1 << SPIF)))
		;

	while (read_len < n) {
		SPDR = 0;		
		while (!(SPSR & (1 << SPIF)))
			;
		buf[read_len++] = SPDR;
	}

	SPI_PORT |= (1 << SPI_SS);

	set_mode(mode);
	return read_len;
}

void radio_listen(void)
{
}

void radio_init(const struct radio_cfg *cfg)
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);

	NRF24L01_CE_DDR |= (1 << NRF24L01_CE);
	NRF24L01_CE_PORT &= ~(1 << NRF24L01_CE);

	_delay_ms(100);
}
