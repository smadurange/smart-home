#include <stdlib.h>

#include <avr/io.h>
#include <util/delay.h>

#include "uart.h"

int main(void)
{
	char s[6];
	unsigned long v;

	uart_init();

	ADMUX |= (1 << REFS0);                               /* AVCC as reference  */
	ADMUX |= (1 << MUX3) | (1 << MUX2) | (1 << MUX1);    /* measure 1.1V VBG */
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS0); /* prescaler 1/32 */

	_delay_us(500);

	for (;;) {
		ADCSRA |= (1 << ADSC);
		while (ADCSRA & (1 << ADSC))
			; 
		v = (1100UL * 1023 / ADC);

		for (int i = 0; i < 6; i++)
			s[i] = 0;
		itoa(v, s, 10);

		uart_write("voltage: ");
		uart_write_line(s);
		_delay_ms(1500);
	}

	return 0;
}
