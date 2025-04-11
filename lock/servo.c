/* Lock back, connected to the servo */

#include <stdint.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

#define PWM_MIN      500
#define PWM_MID      1500
#define PWM_MAX      2500
#define PWM_TOP      20000

#define SERVO_PIN    PB1
#define LOCK_BTN     PD6
#define UNLOCK_BTN   PD7

#define RX_PIN       PD7
#define RX_DDR       DDRD
#define RX_PORT      PORTD
#define RX_PCIE      PCIE2
#define RX_PCINT     PCINT23
#define RX_PCMSK     PCMSK2
#define RX_PCINTVEC  PCINT2_vect

static volatile uint8_t rxdr = 0;

static inline void servo_init(void)
{
	DDRB |= (1 << SERVO_PIN);

	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM13) | (1 << CS11);

	ICR1 = PWM_TOP;

	DDRD &= ~((1 << LOCK_BTN) | (1 << UNLOCK_BTN));
	PORTD |= (1 << LOCK_BTN) | (1 << UNLOCK_BTN);
}

int main(void)
{
	uint8_t n;
	uint8_t rxaddr[ADDRLEN] = { 194, 178, 83 };

	char buf[WDLEN + 1], key[WDLEN + 1], msg[WDLEN + 1];

	RX_DDR &= ~(1 << RX_PIN);
	RX_PORT |= (1 << RX_PIN); 
	PCICR |= (1 << RX_PCIE);
	RX_PCMSK |= (1 << RX_PCINT);

	wdt_init();
	uart_init();
	servo_init();
	radio_init(rxaddr);
	radio_print_config();

	sei();
	radio_listen();

	for (;;) {
		wdt_reset();
		if (rxdr) {
		} else {
			_delay_ms(2000);
		}
	}
	return 0;
}

ISR(RX_PCINTVEC)
{
	rxdr = 1;
}
