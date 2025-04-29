#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "uart.h"

#define LOCK_PIN      PD2
#define UNLOCK_PIN    PD3

static inline void init_wdt(void)
{
	cli();
	wdt_reset();
	WDTCSR |= (1 << WDCE) | ( 1 << WDE); 
	WDTCSR = (1 << WDP2) | (1 << WDP1) | (1 << WDP0);
	WDTCSR |= (1 << WDIE);
}

static inline void init_btns(void)
{
	DDRD &= ~((1 << LOCK_PIN) | (1 << UNLOCK_PIN));
	PORTD |= ((1 << LOCK_PIN) | (1 << UNLOCK_PIN));
	EICRA = 0b00000000;
	EIMSK = (1 << INT0) | (1 << INT1);
}

int main(void)
{
	init_wdt();
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
		_delay_ms(500); /* wait for start-up */
		uart_write_line("Doing some work...");
		_delay_ms(500);
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

ISR(WDT_vect)
{
}
