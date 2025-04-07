/* Door back, connected to the fingerprint scanner */

#include <stdint.h>
#include <string.h>

#include <avr/interrupt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

#define RX_PIN         PD7
#define RX_DDR         DDRD
#define RX_PORT        PORTD
#define RX_PCIE        PCIE2
#define RX_PCINT       PCINT23
#define RX_PCMSK       PCMSK2
#define RX_PCINTVEC    PCINT2_vect

// todo: atomic var
static volatile int rxdr = 0;

static inline void await_reply(void)
{
	uint8_t i;

	radio_listen();
	for (i = 0; i < 500 && rxdr == 0; i += 100)
		_delay_ms(100);
}

int main(void)
{
	uint8_t rxaddr[ADDRLEN] = { 194, 178, 83 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 82 };

	char buf[WDLEN + 1];
	char key[WDLEN + 1];
	char msg[WDLEN + 1];

	RX_DDR &= ~(1 << RX_PIN);
	RX_PORT |= (1 << RX_PIN); 
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);

	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	sei();
	radio_listen();

	for (;;) {
		if (rxdr) {
			n = radio_recv(buf, WDLEN);
			buf[n] = '\0';
			rxdr = 0;
			xor(KEY, buf, msg, WDLEN);
			if (strncmp(msg, SYN, WDLEN) == 0) {
				xor(KEY, key, msg, WDLEN);
				radio_sendto(txaddr, msg, WDLEN);
				await_reply();
				if (rxdr) {
					n = radio_recv(buf, WDLEN);
					buf[n] = '\0';
					rxdr = 0;
					xor(key, buf, msg, WDLEN);
					if (strncmp(msg, LOCK, WDLEN) == 0) {
						uart_write_line("LOCKED");
					} else if (strncmp(msg, UNLOCK, WDLEN) == 0) {
						uart_write_line("UNLOCKED");
					} else {
						uart_write("ERROR: unknown message ");
						uart_write_line(msg);
					}
				}
			} else {
				uart_write_line("ERROR: handshake failed");
			}
		} else {
			uart_write_line("No IRQ");
			_delay_ms(2000);
		}
	}
	return 0;
}

ISR(RX_PCINTVEC)
{
	rxdr = 1;
}
