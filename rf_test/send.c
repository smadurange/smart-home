#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "radio.h"
#include "serial.h"

int main(void)
{
	struct radio_cfg cfg;
	const char *s = "hello, world!";

	cfg.payload_len = PAYLOAD_LEN;

	serial_init();
	radio_init(&cfg);

	sei();

	for (;;) {
		radio_send(s, strlen(s));
		serial_write_line("sent data");
		_delay_ms(2000);
	}

	return 0;
}

