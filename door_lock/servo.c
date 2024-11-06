#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "cmd.h"
#include "serial.h"

#define PWM_MIN 1200
#define PWM_MID 3000
#define PWM_MAX 5000

#define SERVO_PIN   PB1
#define LOCK_BTN    PD6
#define UNLOCK_BTN  PD7

static inline void lock(void)
{
	OCR1A = PWM_MID;
}

static inline void unlock(void)
{
	OCR1A = PWM_MIN;
}

static inline int is_btn_pressed(unsigned char btn)
{
    return !((PIND >> btn) & 0x01);
}

static inline void pcint2_init(void)
{
	PCICR |= (1 << PCIE2);
	PCMSK2 |= ((1 << PCINT22) | (1 << PCINT23));
}

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
	char *s;

	servo_init();
	pcint2_init();
	serial_init();

	sei();

	for(;;) {
		s = cmd_hash(DOOR_UNLOCK);
		if (cmd_cmp(s, DOOR_UNLOCK))
			serial_write_line("unlock");
		else
			serial_write_line("do not unlock");

		_delay_ms(1000);
	}

	return 0;
}

ISR(PCINT2_vect)
{
	if (is_btn_pressed(LOCK_BTN))
		lock();

	if (is_btn_pressed(UNLOCK_BTN))
		unlock();
}
