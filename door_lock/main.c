#include <avr/io.h>
#include <util/delay.h>

#include "serial.h"

int main(void) {
	// pin 9
	DDRB |= 1 << PINB1;
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);

	ICR1 = 39999;
	int offset = 800;

	serial_init();

	for(;;) {
		OCR1A = 3999 + offset;
		_delay_ms(5000);

		OCR1A = 1999 - offset;
		_delay_ms(5000);

		serial_write_line("hello, world!");
	}

	return 0;
}
