#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define PWM_MIN 1200
#define PWM_MID 3000
#define PWM_MAX 5000

#define SERVO_PIN  PB1
#define LOCK_BTN   PD6
#define UNLOCK_BTN PD7

static inline void servo_init(void)
{
	DDRB |= 1 << SERVO_PIN;
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);

	ICR1 = 40000;

	DDRD &= ~((1 << LOCK_BTN) | (1 << UNLOCK_BTN));
	PORTD |= (1 << LOCK_BTN) | (1 << UNLOCK_BTN);
}

int main(void) 
{
	servo_init();

	EICRA |= (1 << ISC00);
	EIMSK |= (1 << INT0);

	sei();

	for(;;)
		;

	return 0;
}

ISR (INT0_vect)
{
	if (!((PIND >> LOCK_BTN) & 0x01))
		OCR1A = PWM_MID;

	if (!((PIND >> UNLOCK_BTN) & 0x01))
		OCR1A = PWM_MIN;
}
