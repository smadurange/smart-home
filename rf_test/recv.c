#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "radio.h"
#include "serial.h"

#define RX_PIN         PB0
#define RX_DDR         DDRB
#define RX_PORT        PORTB
#define RX_PCIE        PCIE0
#define RX_PCINT       PCINT0
#define RX_PCMSK       PCMSK0
#define RX_PCINTVEC    PCINT0_vect

#define MAX_PAYLOAD_LEN 60

static char *s = "hello, world!";
static uint8_t slen = strlen(s);

int main(void)
{
	struct radio_cfg cfg;

	cfg.netid = 0x01;
	cfg.nodeid = 0x01;
	cfg.payload_len = slen;

	RX_DDR &= ~(1 << RX_PIN);
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);

	serial_init();
	sei();

	radio_init(&cfg);
	radio_listen();

	for (;;)
		;

	return 0;
}

ISR(RX_PCINTVEC)
{
	uint8_t i, n;
	char buf[MAX_PAYLOAD_LEN];

	cli();

	serial_write_line("Detected pin change IRQ");

	n = radio_recv(buf, MAX_PAYLOAD_LEN - 1);
	buf[n] = '\0';
	
	for (i = 0; i < n; i++)
		serial_write(buf[i]);
	serial_write('\r');
	serial_write('\n');

	sei();
}
