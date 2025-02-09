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

#define NRF24L01_POWER_ON_RST_DELAY    100

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

#define NRF24L01_NOP                  0xFF
#define NRF24L01_R_REGISTER           0x00
#define NRF24L01_W_REGISTER           0x20
#define NRF24L01_R_RX_PAYLOAD         0x60
#define NRF24L01_W_TX_PAYLOAD         0xA0
#define NRF24L01_FLUSH_TX             0xE1
#define NRF24L01_FLUSH_RX             0xE2

#define NRF24L01_MASK_RX_DR              6
#define NRF24L01_MASK_TX_DS              5
#define NRF24L01_MASK_MAX_RT             4
#define NRF24L01_PWR_UP                  1
#define NRF24L01_PRIM_RX                 0
#define NRF24L01_ERX_P1                  1
#define NRF24L01_ERX_P0                  0

static inline uint8_t read_reg(uint8_t reg)
{
	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = NRF24L01_R_REGISTER | reg;
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
	SPDR = NRF24L01_W_REGISTER | reg;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = val;
	while (!(SPSR & (1 << SPIF)))
		;
	SPI_PORT |= (1 << SPI_SS);
}

void radio_send(const char *data, uint8_t n)
{
}

uint8_t radio_recv(char *buf, uint8_t n)
{
}

void radio_listen(void)
{
}

void radio_init(const struct radio_cfg *cfg)
{
	uint8_t conf;

	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);

	NRF24L01_CE_DDR |= (1 << NRF24L01_CE);
	NRF24L01_CE_PORT &= ~(1 << NRF24L01_CE);

	_delay_ms(NRF24L01_POWER_ON_RST_DELAY);

	// disable IRQs, set CRC encoding scheme to 2 bytes
	conf = 0;
	conf |= (1 << NRF24L01_MASK_RX_DR);
	conf |= (1 << NRF24L01_MASK_TX_DS);
	conf |= (1 << NRF24L01_MASK_MAX_RT);
	conf |= (1 << NRF24L01_CRCO);
	write_reg(NRF24L01_REG_CONFIG, conf);

	// only use data pipe 0
	conf = 0;
	conf &= ~(1 << NRF24L01_ERX_P1);
	conf |= (1 << NRF24L01_ERX_P0);
	write_reg(NRF24L01_REG_EN_RXADDR, conf);

	// set address width to 3 bytes
	write_reg(NRF24L01_REG_SETUP_AW, 0x01);	

	
}
