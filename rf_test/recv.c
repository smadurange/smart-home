#include <avr/io.h>
#include <avr/interrupt.h>

#include "radio.h"
#include "serial.h"

#define PAYLOAD_LEN    13

#define RX_PIN         PB0
#define RX_DDR         DDRB
#define RX_PORT        PORTB
#define RX_PCIE        PCIE0
#define RX_PCINT       PCINT0
#define RX_PCMSK       PCMSK0
#define RX_PCINTVEC    PCINT0_vect

int main(void)
{
	struct radio_cfg cfg;

	cfg.payload_len = PAYLOAD_LEN;

	RX_DDR &= ~(1 << RX_PIN);
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);

	serial_init();
	radio_init(&cfg);

	sei();

	for (;;)
		;

	return 0;
}

ISR(RX_PCINTVEC)
{
	uint8_t i, n;
	char buf[PAYLOAD_LEN + 1];

	cli();

	n = radio_recv(buf, PAYLOAD_LEN);
	buf[n] = '\0';
	
	for (i = 0; i < n; i++)
		serial_write(buf[i]);
	serial_write('\r');
	serial_write('\n');

	sei();
}
