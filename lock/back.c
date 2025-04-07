/* Door back, connected to the fingerprint scanner */

#include <stdint.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

#define PWM_MIN      500
#define PWM_MID      1500
#define PWM_MAX      2500
#define PWM_TOP      20000

#define RX_PIN       PD7
#define RX_DDR       DDRD
#define RX_PORT      PORTD
#define RX_PCIE      PCIE2
#define RX_PCINT     PCINT23
#define RX_PCMSK     PCMSK2
#define RX_PCINTVEC  PCINT2_vect

static char chars[] = {
	'0', '8', '3', '6', 'a', 'Z', '$', '4', 'v', 'R', '@',
	'E', '1', 'o', '#', ')', '2', '5', 'q', ';', '.', 'I',
	'c', '7', '9', '*', 'L', 'V', '&', 'k', 'K', '!', 'm',
	'N', '(', 'O', 'Q', 'A', '>', 'T', 't', '?', 'S', 'h',
	'w', '/', 'n', 'W', 'l', 'M', 'e', 'H', 'j', 'g', '[',
	'P', 'f', ':', 'B', ']', 'Y', '^', 'F', '%', 'C', 'x'
};

static volatile int rxdr = 0;
static uint8_t charslen = sizeof(chars) / sizeof(chars[0]);

static inline void await_reply(void)
{
	uint8_t i;

	radio_listen();
	for (i = 0; i < 500 && rxdr == 0; i += 100)
		_delay_ms(100);
}

static inline void keygen(char *buf, uint8_t n)
{
	int i;
	uint8_t sreg;
	uint16_t idx; 
	
	sreg = SREG;
	cli();
	idx = TCNT1; 
	SREG = sreg;

	for (i = 0; i < n; i++, idx++)
		buf[i] = chars[(idx % charslen)]  ;
	buf[n - 1] = '\0';
}

static inline void timer_init(void)
{
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM13) | (1 << CS11);
	ICR1 = PWM_TOP; /* we use the same timer for servo's PWM */
}

int main(void)
{
	uint8_t n;
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
	timer_init();
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
				keygen(key, WDLEN);
				uart_write("DEBUG: session key = ");
				uart_write_line(key);

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
			uart_write_line("DEBUG: no new packets");
			_delay_ms(2000);
		}
	}
	return 0;
}

ISR(RX_PCINTVEC)
{
	rxdr = 1;
}
