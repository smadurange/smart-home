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

// Register names
#define RH_RF69_REG_00_FIFO                                 0x00
#define RH_RF69_REG_01_OPMODE                               0x01
#define RH_RF69_REG_02_DATAMODUL                            0x02
#define RH_RF69_REG_03_BITRATEMSB                           0x03
#define RH_RF69_REG_04_BITRATELSB                           0x04
#define RH_RF69_REG_05_FDEVMSB                              0x05
#define RH_RF69_REG_06_FDEVLSB                              0x06
#define RH_RF69_REG_07_FRFMSB                               0x07
#define RH_RF69_REG_08_FRFMID                               0x08
#define RH_RF69_REG_09_FRFLSB                               0x09
#define RH_RF69_REG_0A_OSC1                                 0x0a
#define RH_RF69_REG_0B_AFCCTRL                              0x0b
#define RH_RF69_REG_0C_RESERVED                             0x0c
#define RH_RF69_REG_0D_LISTEN1                              0x0d
#define RH_RF69_REG_0E_LISTEN2                              0x0e
#define RH_RF69_REG_0F_LISTEN3                              0x0f
#define RH_RF69_REG_10_VERSION                              0x10
#define RH_RF69_REG_11_PALEVEL                              0x11
#define RH_RF69_REG_12_PARAMP                               0x12
#define RH_RF69_REG_13_OCP                                  0x13
#define RH_RF69_REG_14_RESERVED                             0x14
#define RH_RF69_REG_15_RESERVED                             0x15
#define RH_RF69_REG_16_RESERVED                             0x16
#define RH_RF69_REG_17_RESERVED                             0x17
#define RH_RF69_REG_18_LNA                                  0x18
#define RH_RF69_REG_19_RXBW                                 0x19
#define RH_RF69_REG_1A_AFCBW                                0x1a
#define RH_RF69_REG_1B_OOKPEAK                              0x1b
#define RH_RF69_REG_1C_OOKAVG                               0x1c
#define RH_RF69_REG_1D_OOKFIX                               0x1d
#define RH_RF69_REG_1E_AFCFEI                               0x1e
#define RH_RF69_REG_1F_AFCMSB                               0x1f
#define RH_RF69_REG_20_AFCLSB                               0x20
#define RH_RF69_REG_21_FEIMSB                               0x21
#define RH_RF69_REG_22_FEILSB                               0x22
#define RH_RF69_REG_23_RSSICONFIG                           0x23
#define RH_RF69_REG_24_RSSIVALUE                            0x24
#define RH_RF69_REG_25_DIOMAPPING1                          0x25
#define RH_RF69_REG_26_DIOMAPPING2                          0x26
#define RH_RF69_REG_27_IRQFLAGS1                            0x27
#define RH_RF69_REG_28_IRQFLAGS2                            0x28
#define RH_RF69_REG_29_RSSITHRESH                           0x29
#define RH_RF69_REG_2A_RXTIMEOUT1                           0x2a
#define RH_RF69_REG_2B_RXTIMEOUT2                           0x2b
#define RH_RF69_REG_2C_PREAMBLEMSB                          0x2c
#define RH_RF69_REG_2D_PREAMBLELSB                          0x2d
#define RH_RF69_REG_2E_SYNCCONFIG                           0x2e
#define RH_RF69_REG_2F_SYNCVALUE1                           0x2f
// another 7 sync word bytes follow, 30 through 36 inclusive
#define RH_RF69_REG_37_PACKETCONFIG1                        0x37
#define RH_RF69_REG_38_PAYLOADLENGTH                        0x38
#define RH_RF69_REG_39_NODEADRS                             0x39
#define RH_RF69_REG_3A_BROADCASTADRS                        0x3a
#define RH_RF69_REG_3B_AUTOMODES                            0x3b
#define RH_RF69_REG_3C_FIFOTHRESH                           0x3c
#define RH_RF69_REG_3D_PACKETCONFIG2                        0x3d
#define RH_RF69_REG_3E_AESKEY1                              0x3e
// Another 15 AES key bytes follow
#define RH_RF69_REG_4E_TEMP1                                0x4e
#define RH_RF69_REG_4F_TEMP2                                0x4f
#define RH_RF69_REG_58_TESTLNA                              0x58
#define RH_RF69_REG_5A_TESTPA1                              0x5a
#define RH_RF69_REG_5C_TESTPA2                              0x5c
#define RH_RF69_REG_6F_TESTDAGC                             0x6f
#define RH_RF69_REG_71_TESTAFC                              0x71

// RH_RF69_REG_01_OPMODE
#define RH_RF69_OPMODE_SEQUENCEROFF                         0x80
#define RH_RF69_OPMODE_LISTENON                             0x40
#define RH_RF69_OPMODE_LISTENABORT                          0x20
#define RH_RF69_OPMODE_MODE                                 0x1c
#define RH_RF69_OPMODE_MODE_SLEEP                           0x00
#define RH_RF69_OPMODE_MODE_STDBY                           0x04
#define RH_RF69_OPMODE_MODE_FS                              0x08
#define RH_RF69_OPMODE_MODE_TX                              0x0c
#define RH_RF69_OPMODE_MODE_RX                              0x10

// RH_RF69_REG_02_DATAMODUL
#define RH_RF69_DATAMODUL_DATAMODE                          0x60
#define RH_RF69_DATAMODUL_DATAMODE_PACKET                   0x00
#define RH_RF69_DATAMODUL_DATAMODE_CONT_WITH_SYNC           0x40
#define RH_RF69_DATAMODUL_DATAMODE_CONT_WITHOUT_SYNC        0x60
#define RH_RF69_DATAMODUL_MODULATIONTYPE                    0x18
#define RH_RF69_DATAMODUL_MODULATIONTYPE_FSK                0x00
#define RH_RF69_DATAMODUL_MODULATIONTYPE_OOK                0x08
#define RH_RF69_DATAMODUL_MODULATIONSHAPING                 0x03
#define RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_NONE        0x00
#define RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0       0x01
#define RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_BT0_5       0x02
#define RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_BT0_3       0x03
#define RH_RF69_DATAMODUL_MODULATIONSHAPING_OOK_NONE        0x00
#define RH_RF69_DATAMODUL_MODULATIONSHAPING_OOK_BR          0x01
#define RH_RF69_DATAMODUL_MODULATIONSHAPING_OOK_2BR         0x02

// RH_RF69_REG_11_PALEVEL
#define RH_RF69_PALEVEL_PA0ON                               0x80
#define RH_RF69_PALEVEL_PA1ON                               0x40
#define RH_RF69_PALEVEL_PA2ON                               0x20
#define RH_RF69_PALEVEL_OUTPUTPOWER                         0x1f

// RH_RF69_REG_23_RSSICONFIG
#define RH_RF69_RSSICONFIG_RSSIDONE                         0x02
#define RH_RF69_RSSICONFIG_RSSISTART                        0x01

// RH_RF69_REG_25_DIOMAPPING1
#define RH_RF69_DIOMAPPING1_DIO0MAPPING                     0xc0
#define RH_RF69_DIOMAPPING1_DIO0MAPPING_00                  0x00
#define RH_RF69_DIOMAPPING1_DIO0MAPPING_01                  0x40
#define RH_RF69_DIOMAPPING1_DIO0MAPPING_10                  0x80
#define RH_RF69_DIOMAPPING1_DIO0MAPPING_11                  0xc0

#define RH_RF69_DIOMAPPING1_DIO1MAPPING                     0x30
#define RH_RF69_DIOMAPPING1_DIO1MAPPING_00                  0x00
#define RH_RF69_DIOMAPPING1_DIO1MAPPING_01                  0x10
#define RH_RF69_DIOMAPPING1_DIO1MAPPING_10                  0x20
#define RH_RF69_DIOMAPPING1_DIO1MAPPING_11                  0x30

#define RH_RF69_DIOMAPPING1_DIO2MAPPING                     0x0c
#define RH_RF69_DIOMAPPING1_DIO2MAPPING_00                  0x00
#define RH_RF69_DIOMAPPING1_DIO2MAPPING_01                  0x04
#define RH_RF69_DIOMAPPING1_DIO2MAPPING_10                  0x08
#define RH_RF69_DIOMAPPING1_DIO2MAPPING_11                  0x0c

#define RH_RF69_DIOMAPPING1_DIO3MAPPING                     0x03
#define RH_RF69_DIOMAPPING1_DIO3MAPPING_00                  0x00
#define RH_RF69_DIOMAPPING1_DIO3MAPPING_01                  0x01
#define RH_RF69_DIOMAPPING1_DIO3MAPPING_10                  0x02
#define RH_RF69_DIOMAPPING1_DIO3MAPPING_11                  0x03

// RH_RF69_REG_26_DIOMAPPING2
#define RH_RF69_DIOMAPPING2_DIO4MAPPING                     0xc0
#define RH_RF69_DIOMAPPING2_DIO4MAPPING_00                  0x00
#define RH_RF69_DIOMAPPING2_DIO4MAPPING_01                  0x40
#define RH_RF69_DIOMAPPING2_DIO4MAPPING_10                  0x80
#define RH_RF69_DIOMAPPING2_DIO4MAPPING_11                  0xc0

#define RH_RF69_DIOMAPPING2_DIO5MAPPING                     0x30
#define RH_RF69_DIOMAPPING2_DIO5MAPPING_00                  0x00
#define RH_RF69_DIOMAPPING2_DIO5MAPPING_01                  0x10
#define RH_RF69_DIOMAPPING2_DIO5MAPPING_10                  0x20
#define RH_RF69_DIOMAPPING2_DIO5MAPPING_11                  0x30

#define RH_RF69_DIOMAPPING2_CLKOUT                          0x07
#define RH_RF69_DIOMAPPING2_CLKOUT_FXOSC_                   0x00
#define RH_RF69_DIOMAPPING2_CLKOUT_FXOSC_2                  0x01
#define RH_RF69_DIOMAPPING2_CLKOUT_FXOSC_4                  0x02
#define RH_RF69_DIOMAPPING2_CLKOUT_FXOSC_8                  0x03
#define RH_RF69_DIOMAPPING2_CLKOUT_FXOSC_16                 0x04
#define RH_RF69_DIOMAPPING2_CLKOUT_FXOSC_32                 0x05
#define RH_RF69_DIOMAPPING2_CLKOUT_FXOSC_RC                 0x06
#define RH_RF69_DIOMAPPING2_CLKOUT_FXOSC_OFF                0x07

// RH_RF69_REG_27_IRQFLAGS1
#define RH_RF69_IRQFLAGS1_MODEREADY                         0x80
#define RH_RF69_IRQFLAGS1_RXREADY                           0x40
#define RH_RF69_IRQFLAGS1_TXREADY                           0x20
#define RH_RF69_IRQFLAGS1_PLLLOCK                           0x10
#define RH_RF69_IRQFLAGS1_RSSI                              0x08
#define RH_RF69_IRQFLAGS1_TIMEOUT                           0x04
#define RH_RF69_IRQFLAGS1_AUTOMODE                          0x02
#define RH_RF69_IRQFLAGS1_SYNADDRESSMATCH                   0x01

// RH_RF69_REG_28_IRQFLAGS2
#define RH_RF69_IRQFLAGS2_FIFOFULL                          0x80
#define RH_RF69_IRQFLAGS2_FIFONOTEMPTY                      0x40
#define RH_RF69_IRQFLAGS2_FIFOLEVEL                         0x20
#define RH_RF69_IRQFLAGS2_FIFOOVERRUN                       0x10
#define RH_RF69_IRQFLAGS2_PACKETSENT                        0x08
#define RH_RF69_IRQFLAGS2_PAYLOADREADY                      0x04
#define RH_RF69_IRQFLAGS2_CRCOK                             0x02

// RH_RF69_REG_2E_SYNCCONFIG
#define RH_RF69_SYNCCONFIG_SYNCON                           0x80
#define RH_RF69_SYNCCONFIG_FIFOFILLCONDITION_MANUAL         0x40
#define RH_RF69_SYNCCONFIG_SYNCSIZE                         0x38
#define RH_RF69_SYNCCONFIG_SYNCSIZE_1                       0x00
#define RH_RF69_SYNCCONFIG_SYNCSIZE_2                       0x08
#define RH_RF69_SYNCCONFIG_SYNCSIZE_3                       0x10
#define RH_RF69_SYNCCONFIG_SYNCSIZE_4                       0x18
#define RH_RF69_SYNCCONFIG_SYNCSIZE_5                       0x20
#define RH_RF69_SYNCCONFIG_SYNCSIZE_6                       0x28
#define RH_RF69_SYNCCONFIG_SYNCSIZE_7                       0x30
#define RH_RF69_SYNCCONFIG_SYNCSIZE_8                       0x38
#define RH_RF69_SYNCCONFIG_SYNCSIZE_SYNCTOL                 0x07

// RH_RF69_REG_37_PACKETCONFIG1
#define RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE         0x80
#define RH_RF69_PACKETCONFIG1_DCFREE                        0x60
#define RH_RF69_PACKETCONFIG1_DCFREE_NONE                   0x00
#define RH_RF69_PACKETCONFIG1_DCFREE_MANCHESTER             0x20
#define RH_RF69_PACKETCONFIG1_DCFREE_WHITENING              0x40
#define RH_RF69_PACKETCONFIG1_DCFREE_RESERVED               0x60
#define RH_RF69_PACKETCONFIG1_CRC_ON                        0x10
#define RH_RF69_PACKETCONFIG1_CRCAUTOCLEAROFF               0x08
#define RH_RF69_PACKETCONFIG1_ADDRESSFILTERING              0x06
#define RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE         0x00
#define RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NODE         0x02
#define RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NODE_BC      0x04
#define RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_RESERVED     0x06

// RH_RF69_REG_3B_AUTOMODES
#define RH_RF69_AUTOMODE_ENTER_COND_NONE                    0x00
#define RH_RF69_AUTOMODE_ENTER_COND_FIFO_NOT_EMPTY          0x20
#define RH_RF69_AUTOMODE_ENTER_COND_FIFO_LEVEL              0x40
#define RH_RF69_AUTOMODE_ENTER_COND_CRC_OK                  0x60
#define RH_RF69_AUTOMODE_ENTER_COND_PAYLOAD_READY           0x80
#define RH_RF69_AUTOMODE_ENTER_COND_SYNC_ADDRESS            0xa0
#define RH_RF69_AUTOMODE_ENTER_COND_PACKET_SENT             0xc0
#define RH_RF69_AUTOMODE_ENTER_COND_FIFO_EMPTY              0xe0

#define RH_RF69_AUTOMODE_EXIT_COND_NONE                     0x00
#define RH_RF69_AUTOMODE_EXIT_COND_FIFO_EMPTY               0x04
#define RH_RF69_AUTOMODE_EXIT_COND_FIFO_LEVEL               0x08
#define RH_RF69_AUTOMODE_EXIT_COND_CRC_OK                   0x0c
#define RH_RF69_AUTOMODE_EXIT_COND_PAYLOAD_READY            0x10
#define RH_RF69_AUTOMODE_EXIT_COND_SYNC_ADDRESS             0x14
#define RH_RF69_AUTOMODE_EXIT_COND_PACKET_SENT              0x18
#define RH_RF69_AUTOMODE_EXIT_COND_TIMEOUT                  0x1c

#define RH_RF69_AUTOMODE_INTERMEDIATE_MODE_SLEEP            0x00
#define RH_RF69_AUTOMODE_INTERMEDIATE_MODE_STDBY            0x01
#define RH_RF69_AUTOMODE_INTERMEDIATE_MODE_RX               0x02
#define RH_RF69_AUTOMODE_INTERMEDIATE_MODE_TX               0x03

// RH_RF69_REG_3C_FIFOTHRESH
#define RH_RF69_FIFOTHRESH_TXSTARTCONDITION_NOTEMPTY        0x80
#define RH_RF69_FIFOTHRESH_FIFOTHRESHOLD                    0x7f

// RH_RF69_REG_3D_PACKETCONFIG2
#define RH_RF69_PACKETCONFIG2_INTERPACKETRXDELAY            0xf0
#define RH_RF69_PACKETCONFIG2_RESTARTRX                     0x04
#define RH_RF69_PACKETCONFIG2_AUTORXRESTARTON               0x02
#define RH_RF69_PACKETCONFIG2_AESON                         0x01

// RH_RF69_REG_4E_TEMP1
#define RH_RF69_TEMP1_TEMPMEASSTART                         0x08
#define RH_RF69_TEMP1_TEMPMEASRUNNING                       0x04

// RH_RF69_REG_5A_TESTPA1
#define RH_RF69_TESTPA1_NORMAL                              0x55
#define RH_RF69_TESTPA1_BOOST                               0x5d

// RH_RF69_REG_5C_TESTPA2
#define RH_RF69_TESTPA2_NORMAL                              0x70
#define RH_RF69_TESTPA2_BOOST                               0x7c

// RH_RF69_REG_6F_TESTDAGC
#define RH_RF69_TESTDAGC_CONTINUOUSDAGC_NORMAL              0x00
#define RH_RF69_TESTDAGC_CONTINUOUSDAGC_IMPROVED_LOWBETAON  0x20
#define RH_RF69_TESTDAGC_CONTINUOUSDAGC_IMPROVED_LOWBETAOFF 0x30

enum OpMode { DEFAULT, STDBY, TX, RX };

static enum OpMode mode = DEFAULT;
static power = 0;

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

static inline void set_mode(uint8_t m)
{
	uint8_t opmode;

	opmode = read_reg(RH_RF69_REG_01_OPMODE);
	opmode &= ~RH_RF69_OPMODE_MODE;
	opmode |= (m & RH_RF69_OPMODE_MODE);
	write_reg(RH_RF69_REG_01_OPMODE, opmode);

	while (!(read_reg(RH_RF69_REG_27_IRQFLAGS1) 
	            & RH_RF69_IRQFLAGS1_MODEREADY))
		;
}

static inline void set_mode_stdby(void)
{
	if (mode != STDBY) {
		if (power >= 18) {
			write_reg(RH_RF69_REG_5A_TESTPA1, RH_RF69_TESTPA1_NORMAL);
			write_reg(RH_RF69_REG_5C_TESTPA2, RH_RF69_TESTPA2_NORMAL);
		}

		set_mode(RH_RF69_OPMODE_MODE_STDBY);
		mode = STDBY;
	} 
}

void radio_send(const char *data, uint8_t n)
{
	uint8_t i;

	write_reg(0x01, 0x04);	
	while (!(read_reg(0x27) & 0x80))
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
	while (!(read_reg(0x27) & 0x80))
		;
}

uint8_t radio_recv(char *buf, uint8_t n)
{
	uint8_t read_len;

	read_len = 0;

	write_reg(0x01, 0x04);	
	while (!(read_reg(0x27) & 0x80))
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
	while (!(read_reg(0x27) & 0x80))
		;

	// todo: go to low power mode
}

void radio_init(const struct radio_cfg *cfg)
{
	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);

	set_mode_stdby();

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
