#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "util.h"

int is_btn_pressed(uint8_t pin, uint8_t btn)
{
	if (!((pin >> btn) & 0x01)) {
		_delay_us(2000);
		return !((pin >> btn) & 0x01);
	}
	return 0;
}

void xor(const char *k, const char *s, char *d, uint8_t n)
{
	int i;

	for (i = 0; i < n; i++)
		d[i] = s[i] ^ k[i];
}

