/* Lock back, connected to the servo */

#include <stdint.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

#define PWM_MIN       500
#define PWM_MID       1500
#define PWM_MAX       2500
#define PWM_TOP       20000

#define SERVO_PIN     PB1
#define LOCK_PIN      PD2
#define UNLOCK_PIN    PD3

#define RX_PIN        PD7
#define RX_DDR        DDRD
#define RX_PORT       PORTD
#define RX_PCIE       PCIE2
#define RX_PCINT      PCINT23
#define RX_PCMSK      PCMSK2
#define RX_PCINTVEC   PCINT2_vect

static char tab[] = {
	'0', '8', '3', '6', 'a', 'Z', '$', '4', 'v', 'R', '@',
	'E', '1', 'o', '#', ')', '2', '5', 'q', ';', '.', 'I',
	'c', '7', '9', '*', 'L', 'V', '&', 'k', 'K', '!', 'm',
	'N', '(', 'O', 'Q', 'A', '>', 'T', 't', '?', 'S', 'h',
	'w', '/', 'n', 'W', 'l', 'M', 'e', 'H', 'j', 'g', '[',
	'P', 'f', ':', 'B', ']', 'Y', '^', 'F', '%', 'C', 'x'
};

static uint8_t synced = 0;
static volatile uint8_t rxd = 0;
static volatile uint8_t islock = 0;
static volatile uint8_t isunlock = 0;
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

static inline void init_rx(void)
{
	RX_DDR &= ~(1 << RX_PIN);
	RX_PORT |= (1 << RX_PIN); 
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);
}

static inline void init_btns(void)
{
	DDRD &= ~((1 << LOCK_PIN) | (1 << UNLOCK_PIN));
	PORTD |= ((1 << LOCK_PIN) | (1 << UNLOCK_PIN));

	EICRA = 0b00000000;
	EIMSK = (1 << INT0) | (1 << INT1);
}

static inline void init_servo(void)
{
	DDRB |= (1 << SERVO_PIN);

	ICR1 = PWM_TOP;
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM13) | (1 << CS11);
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

	init_rx();
	init_btns();
	init_servo();

	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	sei();
	radio_listen();

	for (;;) {
		if (!rxd)
			_delay_ms(500);

		if (rxd) {
			n = radio_recv(buf, WDLEN);
			buf[n] = '\0';	
			if (!synced) {
				xor(KEY, buf, msg, WDLEN);
				if (strncmp(msg, SYN, WDLEN) == 0) {
					keygen(key, WDLEN + 1);
					xor(KEY, key, buf, WDLEN);
					synced = radio_sendto(txaddr, buf, WDLEN);
				}
			} else {
				synced = 0;
				xor(key, buf, msg, WDLEN);
				if (strncmp(msg, LOCK, WDLEN) == 0)
					lock();
				else if (strncmp(msg, UNLOCK, WDLEN) == 0)
					unlock();
				keydel(buf, WDLEN);
			}
			rxd = 0;
		}

		if (islock) {
			lock();
			islock = 0;
		} else if (isunlock) {
			unlock();
			isunlock = 0;
		}
	}
	return 0;
}

ISR(RX_PCINTVEC)
{
	rxd = 1;
}

ISR(INT0_vect)
{
	if (is_btn_pressed(PIND, LOCK_PIN))
		islock = 1;
}

ISR(INT1_vect)
{
	if (is_btn_pressed(PIND, UNLOCK_PIN))
		isunlock = 1;
}
