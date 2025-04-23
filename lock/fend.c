/* Lock front, connected to the fingerprint scanner */

#include <stdint.h>
#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

#define LOCK_PIN      PD2
#define UNLOCK_PIN    PD3

#define RX_PIN        PD7
#define RX_DDR        DDRD
#define RX_PORT       PORTD
#define RX_PCIE       PCIE2
#define RX_PCINT      PCINT23
#define RX_PCMSK      PCMSK2
#define RX_PCINTVEC   PCINT2_vect

uint8_t sync = 0;
static volatile uint8_t rxd = 0;
static volatile uint8_t islock = 0;
static volatile uint8_t isunlock = 0;

static inline void init_rx(void)
{
	RX_DDR &= ~(1 << RX_PIN);
	RX_PORT |= (1 << RX_PIN); 
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);
}

static inline void init_btns(void)
{
	DDRD &= ~((1 << LOCK_PIN) | (1 << UNLOCK_PIN));
	PORTD |= ((1 << LOCK_PIN) | (1 << UNLOCK_PIN));

	EICRA = 0b00000000;
	EIMSK = (1 << INT0) | (1 << INT1);
}

int main(void)
{
	uint8_t n;
	uint8_t rxaddr[ADDRLEN] = { 194, 178, 82 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 83 };

	char buf[WDLEN + 1], key[WDLEN + 1];

	init_rx();
	init_btns();

	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	sei();

	for (;;) {
		if ((islock || isunlock) && !sync) {
			sync = 1;
			xor(KEY, SYN, buf, WDLEN);
			do {
				sync = radio_sendto(txaddr, buf, WDLEN);
				_delay_ms(10);
			} while (!sync);
			sync = 1;
		}

		if (rxd) {
			if (sync) {
				n = radio_recv(buf, WDLEN);
				buf[n] = '\0';
				xor(KEY, buf, key, WDLEN);
				if (islock) {
					islock = 0;
					xor(key, LOCK, buf, WDLEN);
				} else if (isunlock) {
					isunlock = 0;
					xor(key, UNLOCK, buf, WDLEN);
				}
				radio_sendto(txaddr, buf, WDLEN);
				sync = 0;
			} else {
				radio_flush_rx();
			}
			rxd = 0;
		}
	}
	return 0;
}

ISR(RX_PCINTVEC)
{
	rxd = 1;
}

ISR(INT0_vect)
{
	if (is_btn_pressed(PIND, LOCK_PIN))
		islock = 1;
	uart_write_line(" start lock");
}

ISR(INT1_vect)
{
	if (is_btn_pressed(PIND, UNLOCK_PIN))
		isunlock = 1;
	uart_write_line("start unlock");
}
