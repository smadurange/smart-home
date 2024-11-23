#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LOCK_LED    PB5
#define LOCK_BTN    PD6
#define UNLOCK_BTN  PD7

#define SYN    0xAA
#define LOCK   0xB5
#define UNLOCK 0xAE

#define SIGPIN  PD2
#define SIGLEN  500

static inline void send(unsigned char c)
{
	int n;
	
	for (n = 7; n >= 0; n--) {
		PORTD = ((c >> n) & 1) == 1 
			? PORTD | (1 << SIGPIN) 
			: PORTD & ~(1 << SIGPIN);

		_delay_us(SIGLEN);
	}

	PORTD &= ~(1 << SIGPIN);
}

static inline void lock(void)
{
	send(SYN);
	send(LOCK);
	PORTB |= (1 << LOCK_LED);
}

static inline void unlock(void)
{
	send(SYN);
	send(UNLOCK);
	PORTB &= ~(1 << LOCK_LED);
}

static inline int is_btn_pressed(unsigned char btn)
{
	if (!((PINB >> btn) & 0x01)) {
		_delay_us(2000);
		return !((PINB >> btn) & 0x01);
	}
	
	return 0;
}

static inline void pcint2_init(void)
{
	PCICR |= (1 << PCIE2);
	PCMSK2 |= ((1 << PCINT22) | (1 << PCINT23));
}

int main(void)
{
	DDRB |= (1 << LOCK_LED);

	DDRD &= ~((1 << LOCK_BTN) | (1 << UNLOCK_BTN));
	PORTD |= (1 << LOCK_BTN) | (1 << UNLOCK_BTN);

	DDRD |= (1 << SIGPIN);
	PORTD &= ~(1 << SIGPIN);

	pcint2_init();
	sei();

	for (;;) {
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
