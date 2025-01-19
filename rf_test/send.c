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

	cfg.netid = 0x01;
	cfg.nodeid = 0x02;
	cfg.payload_len = n;

	serial_init();
	radio_init(&cfg);
	radio_set_tx_power(14);

	for (;;) {
		radio_send(s, n);
		serial_write_line("sent");
		_delay_ms(1500);
	}

	return 0;
}
