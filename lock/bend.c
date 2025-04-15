/* Lock back, connected to the servo */

#include <stdint.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

#define PWM_MIN            500
#define PWM_MID            1500
#define PWM_MAX            2500
#define PWM_TOP            20000

#define SERVO_PIN          PB1

#define BTN_PIN            PINC
#define BTN_PCIE           PCIE1
#define BTN_PCINTVEC       PCINT1_vect
#define LOCK_BTN           PD6
#define UNLOCK_BTN         PD7
#define LOCK_BTN_PCINT     PCINT12
#define UNLOCK_BTN_PCINT   PCINT13

#define RX_PIN             PD7
#define RX_DDR             DDRD
#define RX_PORT            PORTD
#define RX_PCIE            PCIE2
#define RX_PCINT           PCINT23
#define RX_PCMSK           PCMSK2
#define RX_PCINTVEC        PCINT2_vect

static char tab[] = {
	'0', '8', '3', '6', 'a', 'Z', '$', '4', 'v', 'R', '@',
	'E', '1', 'o', '#', ')', '2', '5', 'q', ';', '.', 'I',
	'c', '7', '9', '*', 'L', 'V', '&', 'k', 'K', '!', 'm',
	'N', '(', 'O', 'Q', 'A', '>', 'T', 't', '?', 'S', 'h',
	'w', '/', 'n', 'W', 'l', 'M', 'e', 'H', 'j', 'g', '[',
	'P', 'f', ':', 'B', ']', 'Y', '^', 'F', '%', 'C', 'x'
};

static uint8_t syn = 0;
static volatile uint8_t rxdr = 0;
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

static inline void btn_init(void)
{
	DDRD &= ~((1 << LOCK_BTN) | (1 << UNLOCK_BTN));
	PORTD |= (1 << LOCK_BTN) | (1 << UNLOCK_BTN);
	PCICR |= (1 << BTN_PCIE);
	PCMSK2 |= ((1 << LOCK_BTN_PCINT) | (1 << UNLOCK_BTN_PCINT));
}

static inline void servo_init(void)
{
	DDRB |= (1 << SERVO_PIN);

	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM13) | (1 << CS11);
	ICR1 = PWM_TOP;
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
	uint8_t n;
	uint8_t rxaddr[ADDRLEN] = { 194, 178, 83 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 82 };

	char buf[WDLEN + 1], key[WDLEN + 1], msg[WDLEN + 1];

	RX_DDR &= ~(1 << RX_PIN);
	RX_PORT |= (1 << RX_PIN); 
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);

	uart_init();
	btn_init();
	servo_init();
	radio_init(rxaddr);
	radio_print_config();

	sei();
	radio_listen();

	for (;;) {
		if (!rxdr)
			_delay_ms(500);

		if (rxdr) {
			rxdr = 0;
			n = radio_recv(buf, WDLEN);
			buf[n] = '\0';	
			if (!syn) {
				xor(KEY, buf, msg, WDLEN);
				if (strncmp(msg, SYN, WDLEN) == 0) {
					syn = 1;
					keygen(key, WDLEN + 1);
					xor(KEY, key, buf, WDLEN);
					radio_sendto(txaddr, buf, WDLEN);
				}
			} else {
				syn = 0;
				xor(key, buf, msg, WDLEN);
				if (strncmp(msg, LOCK, WDLEN) == 0)
					lock();
				else if (strncmp(msg, UNLOCK, WDLEN) == 0)
					unlock();
				keydel(buf, WDLEN);
			}
		}

		// todo: sleep
	}
	return 0;
}

ISR(RX_PCINTVEC)
{
	rxdr = 1;
}

ISR(BTN_PCINTVEC)
{
	if (is_btn_pressed(BTN_PIN, LOCK_BTN))
		lock();
	else if (is_btn_pressed(BTN_PIN, UNLOCK_BTN))
		unlock();
}
