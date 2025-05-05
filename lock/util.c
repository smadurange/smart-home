#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "util.h"

#define LOCK_LED      PC3
#define UNLOCK_LED    PC4
#define BATLOW_LED    PC5
#define LED_DDR       DDRC
#define LED_PORT      PORTC

int is_btn_pressed(uint8_t pin, uint8_t btn)
{
	if (!((pin >> btn) & 0x01)) {
		_delay_ms(100);
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

/* Measure vcc by measuring known internal 1.1v bandgap
 * reference voltage against AVCC.
 * Place a 100nF bypass capacitor on AREF.
 */
uint16_t getvcc(void)
{
	uint16_t vcc;

	ADMUX |= (1 << REFS0);
	ADMUX |= (1 << MUX3) | (1 << MUX2) | (1 << MUX1);
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS0);

	// https://www.sciencetronics.com/greenphotons/?p=1521
	_delay_us(500);

	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC))
		; 
	vcc = (1100UL * 1023 / ADC);

	ADCSRA &= ~(1 << ADEN);
	return vcc;
}

void led_init(void) 
{
	LED_DDR |= (1 << LOCK_LED) | (1 << UNLOCK_LED);
	LED_DDR |= (1 << BATLOW_LED);

	LED_PORT &= ~(1 << LOCK_LED);
	LED_PORT &= ~(1 << UNLOCK_LED);
	LED_PORT &= ~(1 << BATLOW_LED);
}

void led_locked(void)
{
	LED_PORT |= (1 << LOCK_LED);
	_delay_ms(70);
	LED_PORT &= ~(1 << LOCK_LED);
	_delay_ms(70);
	LED_PORT |= (1 << LOCK_LED);
	_delay_ms(70);
	LED_PORT &= ~(1 << LOCK_LED);
}

void led_unlocked(void)
{
	LED_PORT |= (1 << UNLOCK_LED);
	_delay_ms(70);
	LED_PORT &= ~(1 << UNLOCK_LED);
	_delay_ms(70);
	LED_PORT |= (1 << UNLOCK_LED);
	_delay_ms(70);
	LED_PORT &= ~(1 << UNLOCK_LED);
}

void led_bat(void)
{
	LED_PORT ^= (1 << BATLOW_LED);
}
