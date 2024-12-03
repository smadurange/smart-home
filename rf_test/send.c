#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "radio.h"
#include "serial.h"

int main(void)
{
	uint8_t n;
	struct radio_cfg cfg;
	const char *s = "hello, world!";

	n = strlen(s);
	cfg.payload_len = n;

	serial_init();
	radio_init(&cfg);

	sei();

	for (;;) {
		radio_send(s, n);
		serial_write_line("sent data");
		_delay_ms(2000);
	}

	return 0;
}

