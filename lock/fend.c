/* Lock front, connected to the fingerprint scanner */

#include <stdint.h>
#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

int main(void)
{
	uint8_t n;
	uint8_t rxaddr[ADDRLEN] = { 194, 178, 82 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 83 };

	char buf[WDLEN + 1], msg[WDLEN + 1];

	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	sei();

	for (;;) {
		_delay_ms(2000);
	}

	return 0;
}

