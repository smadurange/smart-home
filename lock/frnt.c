/* Door front, connected to the fingerprint scanner  */

#include <stdint.h>
#include <string.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

#define RX_PIN       PD7
#define RX_DDR       DDRD
#define RX_PORT      PORTD
#define RX_PCIE      PCIE2
#define RX_PCINT     PCINT23
#define RX_PCMSK     PCMSK2
#define RX_PCINTVEC  PCINT2_vect

// todo: atomic var
static volatile int rxdr = 0;

static inline void await_reply(void)
{
	radio_listen();
	for (i = 0; i < 500 && rxdr == 0; i += 100)
		_delay_ms(100);
}

int main(void)
{
	uint8_t i;
	uint8_t rxaddr[ADDRLEN] = { 194, 178, 82 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 83 };

	char key[WDLEN + 1], msg[WDLEN + 1];

	RX_DDR &= ~(1 << RX_PIN);
	RX_PORT |= (1 << RX_PIN); 
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);

	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	for (;;) {
		_delay_ms(2000); /* todo: fingerprint check */
		xor(KEY, SYN, msg, WDLEN);
		radio_sendto(txaddr, msg, WDLEN);
		await_reply();
		if (rxdr) {
			n = radio_recv(msg, WDLEN);
			msg[n] = '\0';
			rxdr = 0;
			xor(KEY, msg, key, WDLEN);
			xor(key, LOCK, msg, WDLEN);
			radio_sendto(txaddr, msg, WDLEN);
			await_reply();
			if (rxdr) {
				n = radio_recv(msg, WDLEN);
				msg[n] = '\0';
				rxdr = 0;
				uart_write_line(msg);
			} else {
				// power down
				uart_write_line("ERROR: no reply");
			}
		} else {
			// power down
			uart_write_line("ERROR: no session key");
		}
	}
	return 0;
}

ISR(RX_PCINTVEC)
{
	rxdr = 1;
}
