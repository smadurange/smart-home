#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TEST_LED    PB1
#define LOCK_LED    PD6
#define UNLOCK_LED  PD7

#define SYN    0xAA
#define LOCK   0xB5
#define UNLOCK 0xAE

#define SIGPIN PB3
#define SIGLEN 200

static volatile unsigned char data = 0;

static inline void led_init(void)
{
	DDRB  |= (1 << TEST_LED);
	DDRD  |= (1 << LOCK_LED) | (1 << UNLOCK_LED);
}

static inline void pcint2_init(void)
{
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT2);
}

int main(void)
{
	DDRB &= ~(1 << SIGPIN);
	PORTB &= ~(1 << SIGPIN);

	led_init();
	pcint2_init();

	sei();

	for (;;) {
		if (data == LOCK) {
			PORTD |= (1 << LOCK_LED);
			PORTD &= ~(1 << UNLOCK_LED);
		}

		if (data == UNLOCK) {
			PORTD &= ~(1 << LOCK_LED);
			PORTD |= (1 << UNLOCK_LED);
		}

		data = 0;
		_delay_ms(100);
	}

	return 0;
}

ISR(PCINT2_vect)
{
	int n, bit;

	for (n = 7; n >= 0; n--) {
		_delay_ms(SIGLEN);
		bit = ((PINB >> SIGPIN) & 1);	
		data = bit == 1 ? (data | (1 << n)) : (data & ~(1 << n));
	}
}
