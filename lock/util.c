#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "util.h"

static char tab[] = {
	'0', '8', '3', '6', 'a', 'Z', '$', '4', 'v', 'R', '@',
	'E', '1', 'o', '#', ')', '2', '5', 'q', ';', '.', 'I',
	'c', '7', '9', '*', 'L', 'V', '&', 'k', 'K', '!', 'm',
	'N', '(', 'O', 'Q', 'A', '>', 'T', 't', '?', 'S', 'h',
	'w', '/', 'n', 'W', 'l', 'M', 'e', 'H', 'j', 'g', '[',
	'P', 'f', ':', 'B', ']', 'Y', '^', 'F', '%', 'C', 'x'
};

static uint16_t tablen = sizeof(tab) / sizeof(tab[0]);

void wdt_init(void)
{
	wdt_reset();
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = (1 << WDE) | (1 << WDP3) | (1 << WDP0);
}

void keygen(char *buf, uint8_t n)
{
	int i, imax;
	uint8_t sreg;
	uint16_t idx; 
	
	sreg = SREG;
	cli();
	idx = TCNT1; 
	SREG = sreg;

	for (i = 0, imax = n - 1; i < imax; i++, idx++)
		buf[i] = tab[(idx % tablen)];
	buf[imax] = '\0';
}

void xor(const char *k, const char *s, char *d, uint8_t n)
{
	int i;

	for (i = 0; i < n; i++)
		d[i] = s[i] ^ k[i];
}

