#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "uart.h"

#define LOCK_PIN      PD2
#define UNLOCK_PIN    PD3

static inline void init_btns(void)
{
	DDRD &= ~((1 << LOCK_PIN) | (1 << UNLOCK_PIN));
	PORTD |= ((1 << LOCK_PIN) | (1 << UNLOCK_PIN));
	EICRA = 0b00000000;
	EIMSK = (1 << INT0) | (1 << INT1);
}

int main(void)
{
	init_btns();
	uart_init();	

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sei();

	for (;;) {
		uart_write_line("Going to sleep");
		sleep_enable();	
		sleep_bod_disable();
		sleep_cpu();
		sleep_disable();
		uart_write_line("It's alive!");
		_delay_ms(1500);
	}
	return 0;
}

static inline int is_btn_pressed(uint8_t pin, uint8_t btn)
{
	if (!((pin >> btn) & 0x01)) {
		_delay_ms(100);
		return !((pin >> btn) & 0x01);
	}
	return 0;
}

ISR(INT0_vect)
{
	if (is_btn_pressed(PIND, LOCK_PIN))
		uart_write_line("Locked");
}

ISR(INT1_vect)
{
	if (is_btn_pressed(PIND, UNLOCK_PIN))
		uart_write_line("Unlocked");
}
