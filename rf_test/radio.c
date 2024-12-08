// **********************************************************************************
// Driver definition for HopeRF RFM69W/RFM69HW/RFM69CW/RFM69HCW, Semtech SX1231/1231H
// **********************************************************************************
// Copyright LowPowerLab LLC 2018, https://www.LowPowerLab.com/contact
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it 
// and/or modify it under the terms of the GNU General    
// Public License as published by the Free Software       
// Foundation; either version 3 of the License, or        
// (at your option) any later version.                    
//                                                        
// This program is distributed in the hope that it will   
// be useful, but WITHOUT ANY WARRANTY; without even the  
// implied warranty of MERCHANTABILITY or FITNESS FOR A   
// PARTICULAR PURPOSE. See the GNU General Public        
// License for more details.                              
//                                                        
// Licence can be viewed at                               
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************

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

#define MAX_PAYLOAD_LEN      61
#define MAX_POWER_LEVEL      23

static uint8_t node_id = 0;
static uint8_t payload_len = 0;

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

static inline uint8_t rssi(void)
{
	return (-1 * (read_reg(0x24) >> 1));
}

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

void radio_sendto(uint8_t addr, const char *data, uint8_t n)
{
	uint8_t i;

	if (n > MAX_PAYLOAD_LEN)
		n = MAX_PAYLOAD_LEN;

	// force-stop rx
	write_reg(0x3D, ((read_reg(0x3D) & 0xFB) | 0x04));
	while (((read_reg(0x01) & 0x1C) == 0x10) && payload_len == 0 && rssi() < -90)
		// todo: read data in fifo
		;

	write_reg(0x01, 0x04);	
	while (!(read_reg(0x27) & 80))
		;

	SPI_PORT &= ~(1 << SPI_SS);
	SPDR = 0x00 | 0x80;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = n + 3;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = addr;
	while (!(SPSR & (1 << SPIF)))
		;
	SPDR = node_id;
	while (!(SPSR & (1 << SPIF)))
		;
	// ctl byte
	SPDR = 0;
	while (!(SPSR & (1 << SPIF)))
		;
	for (i = 0; i < n; i++) {
		SPDR = data[i];
		while (!(SPSR & (1 << SPIF)))
			;
	}
	SPI_PORT |= (1 << SPI_SS);

	write_reg(0x01, ((read_reg(0x01) & 0xE3) | 0x0C));
	write_reg(0x5A, 0x5D);
	write_reg(0x5C, 0x7C);

	while (!(read_reg(0x28) & 0x08))
		;

	serial_write_line("146");

	write_reg(0x01, ((read_reg(0x01) & 0xE3) | 0x04));
	while (!(read_reg(0x27) & 0x80))
		;
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
	node_id = cfg->node_id;

	SPI_DDR |= (1 << SPI_SS) | (1 << SPI_SCK) | (1 << SPI_MOSI);
	SPI_PORT |= (1 << SPI_SS);
	SPCR |= (1 << SPE) | (1 << MSTR);
	
	// todo: do we need this?
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
