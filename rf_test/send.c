#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "spi.h"

#define LOCK_BTN    PD6
#define UNLOCK_BTN  PD7

#define SYN    0xAA
#define LOCK   0xB5
#define UNLOCK 0xAE

static inline void lock(void)
{
	spi_send(SYN);
	spi_send(LOCK);
}

static inline void unlock(void)
{
	spi_send(SYN);
	spi_send(UNLOCK);
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
	DDRD &= ~((1 << LOCK_BTN) | (1 << UNLOCK_BTN));
	PORTD |= (1 << LOCK_BTN) | (1 << UNLOCK_BTN);

	spi_init();
	pcint2_init();

	sei();

	for (;;)
		;

	return 0;
}

ISR(PCINT2_vect)
{
	if (is_btn_pressed(LOCK_BTN))
		lock();

	if (is_btn_pressed(UNLOCK_BTN))
		unlock();
}
