/* Lock back, connected to the servo */

#include <stdint.h>
#include <string.h>

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "util.h"

#define PWM_MIN       500
#define PWM_MID       1500
#define PWM_MAX       2500
#define PWM_TOP       20000

#define SERVO_PIN     PB1
#define LOCK_PIN      PD3
#define UNLOCK_PIN    PD4

#define RX_IRQ_PIN    PC0
#define RX_DDR        DDRC
#define RX_PIN        PINC
#define RX_ICR        PCICR 
#define RX_IE         PCIE1
#define RX_INT        PCINT8
#define RX_MSK        PCMSK1
#define RX_INTVEC     PCINT1_vect

#define VCC_MIN       4000

static char tab[] = {
	'0', '8', '3', '6', 'a', 'Z', '$', '4', 'v', 'R', '@',
	'E', '1', 'o', '#', ')', '2', '5', 'q', ';', '.', 'I',
	'c', '7', '9', '*', 'L', 'V', '&', 'k', 'K', '!', 'm',
	'N', '(', 'O', 'Q', 'A', '>', 'T', 't', '?', 'S', 'h',
	'w', '/', 'n', 'W', 'l', 'M', 'e', 'H', 'j', 'g', '[',
	'P', 'f', ':', 'B', ']', 'Y', '^', 'F', '%', 'C', 'x'
};

static volatile uint8_t rxd = 0;
static uint16_t tablen = sizeof(tab) / sizeof(tab[0]);

static inline void keygen(char *buf, uint8_t n)
{
	int i, imax;
	uint8_t sreg;
	uint16_t seed; 
	
	sreg = SREG;
	cli();
	seed = TCNT1; 
	SREG = sreg;

	for (i = 0, imax = n - 1; i < imax; i++, seed++)
		buf[i] = tab[(seed % tablen)];
	buf[imax] = '\0';
}

static inline void keydel(char *buf, uint8_t n)
{
	int i;

	for (i = 0; i < n; i++)
		buf[i] = 0;
}

static inline void init_wdt(void)
{
	cli();
	wdt_reset();

	WDTCSR |= (1 << WDCE) | ( 1 << WDE); 
	WDTCSR = (1 << WDE) | (1 << WDP2) | (1 << WDP1);
}

static inline void init_rx(void)
{
	RX_DDR &= ~(1 << RX_IRQ_PIN);
	RX_ICR |= (1 << RX_IE);
	RX_MSK |= (1 << RX_INT);
}

static inline void init_btns(void)
{
	DDRD &= ~((1 << LOCK_PIN) | (1 << UNLOCK_PIN));
	PORTD |= ((1 << LOCK_PIN) | (1 << UNLOCK_PIN));

	EICRA = 0b00000000;
	EIMSK = (1 << INT1);

	PCICR |= (1 << PCIE2);
	PCMSK2 |= (1 << PCINT20);
}

static inline void init_servo(void)
{
	ICR1 = PWM_TOP;
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM13) | (1 << CS11);

	DDRB |= (1 << SERVO_PIN);
}

static inline void lock(void)
{
	OCR1A = PWM_MID;
	_delay_ms(100);
	OCR1A = PWM_TOP;
}

static inline void unlock(void)
{
	OCR1A = PWM_MAX - 50;
	_delay_ms(100);
	OCR1A = PWM_TOP;
}

int main(void)
{
	uint8_t rxaddr[ADDRLEN] = { 194, 178, 83 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 82 };

	char buf[WDLEN], key[WDLEN], msg[WDLEN];

	init_wdt();
	init_rx();
	init_btns();
	init_servo();

	led_init();
	radio_init(rxaddr);

	sei();
	radio_listen();

	for (;;) {
		if (!rxd)
			_delay_ms(250);

		if (rxd) {
			radio_recv(buf, WDLEN);
			rxd = 0;
			xor(KEY, buf, msg, WDLEN);
			if (memcmp(msg, SYN, WDLEN) == 0) {
				keygen(key, WDLEN);
				xor(KEY, key, buf, WDLEN);
				radio_sendto(txaddr, buf, WDLEN);
			} else {
				xor(key, buf, msg, WDLEN);
				if (memcmp(msg, LOCK, WDLEN) == 0) {
					lock();
					keydel(key, WDLEN);
				} else if (memcmp(msg, UNLOCK, WDLEN) == 0) {
					unlock();
					keydel(key, WDLEN);
				}
			}
		} else {
			radio_pwr_dwn();
			if (getvcc() < VCC_MIN)
				led_bat();
			sleep_bod_disable();
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_mode();
		}
	}
	return 0;
}

ISR(RX_INTVEC)
{
	if (!(RX_PIN & (1 << RX_IRQ_PIN)))
		rxd = 1;
}

ISR(INT1_vect)
{
	if (is_btn_pressed(PIND, LOCK_PIN)) {
		lock();
		led_locked();
	}
}

ISR(PCINT2_vect)
{
	if (is_btn_pressed(PIND, UNLOCK_PIN)) {
		unlock();
		led_unlocked();
	}
}

