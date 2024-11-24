#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "rfm.h"
#include "serial.h"

#define TEST_LED    PB1
#define LOCK_LED    PD6
#define UNLOCK_LED  PD7

#define SYN    0xAA
#define LOCK   0xB5
#define UNLOCK 0xAE

#define SIGPIN PB0

static inline void led_init(void)
{
	DDRB  |= (1 << TEST_LED);
	DDRD  |= (1 << LOCK_LED) | (1 << UNLOCK_LED);
}

static inline void pcint2_init(void)
{
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT0);
}

int main(void)
{
	DDRB &= ~(1 << SIGPIN);
	PORTB &= ~(1 << SIGPIN);

	led_init();
	serial_init();
	pcint2_init();

	sei();

	for (;;)
		;

	return 0;
}

ISR(PCINT2_vect)
{
	char *s;
	uint8_t buf[2], n;	

	n = rfm_recvfrom(0x00, buf, 2);

	if (buf[1] == LOCK)
		s = "LOCK";
	else if (buf[1] == UNLOCK)
		s = "UNLOCK";
	else
		s = "Garbage";
	
	serial_write_line(s);
}
