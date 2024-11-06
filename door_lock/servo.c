#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"

#define PWM_MIN 1200
#define PWM_MID 3000
#define PWM_MAX 5000

#define SERVO_PIN  PB1
#define LOCK_BTN   PD6
#define ULOCK_BTN  PD7

#define XORLEN     32
#define KEY        "dM>}jdb,6gsnC$J^K 8(I5vyPemPs%;K"
#define ULOCK_FLAG "43iqr5$NB8SpN?Z/52{iVl>o|i!.'dsT"

static inline void servo_init(void)
{
	DDRB |= 1 << SERVO_PIN;
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);

	ICR1 = 40000;

	DDRD &= ~((1 << LOCK_BTN) | (1 << ULOCK_BTN));
	PORTD |= (1 << LOCK_BTN) | (1 << ULOCK_BTN);
}

static inline void pcint2_init(void)
{
	PCICR |= (1 << PCIE2);
	PCMSK2 |= ((1 << PCINT22) | (1 << PCINT23));
}

static inline uint8_t is_btn_pressed(uint8_t btn)
{
    return !((PIND >> btn) & 0x01);
}

static inline void xor(const char *s, char *d, uint8_t n)
{
	int i;

	for (i = 0; i < n && s[i]; i++)
		d[i] = s[i] ^ KEY[i];
}

int main(void) 
{
	char s1[XORLEN];
	char s2[XORLEN + 1]; 

	servo_init();
	pcint2_init();

	serial_init();

	sei();

	for(;;) {
		// encrypt
		xor(ULOCK_FLAG, s1, XORLEN);

		// decrypt
		xor(s1, s2, XORLEN);
		s2[XORLEN] = 0;
		serial_write_line(s2);

		_delay_ms(1000);
	}

	return 0;
}

ISR(PCINT2_vect)
{
	if (is_btn_pressed(LOCK_BTN))
		OCR1A = PWM_MID;

	if (is_btn_pressed(ULOCK_BTN))
		OCR1A = PWM_MIN;
}
