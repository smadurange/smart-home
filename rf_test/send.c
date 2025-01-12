#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "radio.h"
#include "serial.h"

#define LED_PIN  PB1
#define LED_DDR  DDRB
#define LED_PORT PORTB

int main(void)
{
	uint8_t n;
	struct radio_cfg cfg;
	const char *s = "hello, world!";

	n = strlen(s);

	cfg.netid = 0x01;
	cfg.nodeid = 0x02;
	cfg.payload_len = n;

	LED_DDR |= (1 << LED_PIN);

	serial_init();
	radio_init(&cfg);

	for (;;) {
		radio_send(s, n);
		serial_write_line("sent");
		LED_PORT |= (1 << LED_PIN);
		_delay_ms(100);
		LED_PORT &= ~(1 << LED_PIN);
		_delay_ms(1900);
	}

	return 0;
}
