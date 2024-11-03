#include <avr/io.h>
#include <util/delay.h>

#include "serial.h"

#define PWM_MIN 1200
#define PWM_MID 3000
#define PWM_MAX 5000

int main(void) {
	// pin 9
	DDRB |= 1 << PINB1;
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);

	ICR1 = 40000;

	serial_init();

	for(;;) {
		OCR1A = PWM_MID;
		_delay_ms(5000);
		OCR1A = PWM_MIN;
		_delay_ms(5000);
		OCR1A = PWM_MID;
		_delay_ms(5000);
		OCR1A = PWM_MAX;
		_delay_ms(5000);

		serial_write_line("hello, world!");
	}

	return 0;
}
