/* Lock front, connected to the fingerprint scanner */

#include <stdint.h>
#include <stdlib.h>

#include <avr/interrupt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

int main(void)
{
	uint8_t n;
	uint8_t rxaddr[ADDRLEN] = { 194, 178, 82 };

	char buf[WDLEN + 1], msg[WDLEN + 1];

	/* timer for keygen */
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM13) | (1 << CS11);
	ICR1 = 20000;

	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	sei();

	for (;;) {
	}

	return 0;
}

