#include <avr/io.h>
#include <avr/interrupt.h>

int main(void)
{
	DDRB |= (1 << DDB5);

	TCCR1A = 0;
	TCNT1 = 65535 - (F_CPU / 1024);
	TCCR1B = (1 << CS10) | (1 << CS12);
	TIMSK1 = (1 << TOIE1);

	sei();

	for (;;)
		;

	return 0;
}

ISR(TIMER1_OVF_vect)
{
	PORTB ^= (1 << 5);
	TCNT1 = 65535 - (F_CPU / 1024);
}
