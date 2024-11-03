#include <avr/io.h>
#include <util/setbaud.h>

#include "serial.h"

void serial_init(void)
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
#if USE_2X
	UCSR0A |= (1 << U2X0);
#else
	UCSR0A &= ~(1 << U2X0);
#endif
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void serial_write_raw(unsigned char data)
{
	while (!(UCSR0A & (1 << UDRE0)))
		;
	UDR0 = data;
}

void serial_write(const char *s)
{
	for (; *s; s++)
		serial_write_raw(*s);
	serial_write_raw('\r');
	serial_write_raw('\n');
}
