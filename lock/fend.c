/* Lock front, connected to the fingerprint scanner */

#include <stdint.h>
#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

#define BTN_PIN            PINB
#define BTN_PCIE           PCIE0
#define BTN_PCINTVEC       PCINT0_vect
#define LOCK_BTN           PB0
#define LOCK_BTN_PCINT     PCINT0

#define RX_PIN             PD7
#define RX_DDR             DDRD
#define RX_PORT            PORTD
#define RX_PCIE            PCIE2
#define RX_PCINT           PCINT23
#define RX_PCMSK           PCMSK2
#define RX_PCINTVEC        PCINT2_vect

uint8_t syncing = 0;
static volatile uint8_t rxdr = 0;
static volatile uint8_t btn_press = 0;

int main(void)
{
	uint8_t n, rc;
	uint8_t rxaddr[ADDRLEN] = { 194, 178, 82 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 83 };

	char buf[WDLEN + 1], key[WDLEN + 1];

	RX_DDR &= ~(1 << RX_PIN);
	RX_PORT |= (1 << RX_PIN); 
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);

	uart_init();
	btn_init();
	radio_init(rxaddr);
	radio_print_config();

	sei();

	for (;;) {
		if (rxdr) {
			rxdr = 0;
			if (syncing) {
				n = radio_recv(buf, WDLEN);
				buf[n] = '\0';
				xor(KEY, buf, key, WDLEN);
			}
		}

		if (btn_press) {
			syncing = 1;
			btn_press = 0;
			xor(KEY, SYN, buf, WDLEN);
			do {
				rc = radio_sendto(txaddr, buf, WDLEN);
			} while (!rc);
		}
	}
	return 0;
}

ISR(RX_PCINTVEC)
{
	rxdr = 1;
}

ISR(BTN_PCINTVEC)
{
	btn_press = 1;
}
