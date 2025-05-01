/* Lock front, connected to the fingerprint scanner */

#include <stdint.h>
#include <stdlib.h>

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "nrfm.h"
#include "uart.h"
#include "util.h"

#define LOCK_PIN      PD2
#define UNLOCK_PIN    PD3

#define RX_PIN        PD7
#define RX_DDR        DDRD
#define RX_PORT       PORTD
#define RX_PCIE       PCIE2
#define RX_PCINT      PCINT23
#define RX_PCMSK      PCMSK2
#define RX_PCINTVEC   PCINT2_vect

#define VCC_MIN       4000

#define LOCK_LED      PC3
#define UNLOCK_LED    PC4
#define BATLOW_LED    PC5
#define LED_DDR       DDRC
#define LED_PORT      PORTC

static volatile uint8_t rxd = 0;
static volatile uint8_t sync = 0;
static volatile uint8_t islock = 0;
static volatile uint8_t isunlock = 0;

static inline void wdt_off(void)
{
	cli();
	wdt_reset();
	MCUSR &= ~(1 << WDRF);
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = 0x00;
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

static inline void init_leds(void) 
{
	LED_DDR |= (1 << LOCK_LED) | (1 << UNLOCK_LED);
	LED_DDR |= (1 << BATLOW_LED);

	LED_PORT &= ~(1 << LOCK_LED);
	LED_PORT &= ~(1 << UNLOCK_LED);
	LED_PORT &= ~(1 << BATLOW_LED);
}

int main(void)
{
	int i;
	uint8_t rxaddr[ADDRLEN] = { 194, 178, 82 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 83 };

	char buf[WDLEN], key[WDLEN];

	wdt_off();
	init_rx();
	init_leds();
	init_btns();

	uart_init();
	radio_init(rxaddr);
	radio_print_config();

	sei();
	radio_listen();

	for (;;) {
		if (!sync && (islock || isunlock)) {
			xor(KEY, SYN, buf, WDLEN);
			do {
				sync = radio_sendto(txaddr, buf, WDLEN);
				_delay_ms(50);
			} while (!sync);
		}

		if (rxd) {
			radio_recv(buf, WDLEN);
			rxd = 0;
			if (sync && (islock || isunlock)) {
				sync = 0;
				xor(KEY, buf, key, WDLEN);
				if (islock) {
					islock = 0;
					xor(key, LOCK, buf, WDLEN);
				} else if (isunlock) {
					isunlock = 0;
					xor(key, UNLOCK, buf, WDLEN);
				}
				radio_sendto(txaddr, buf, WDLEN);
			}
		}

		if (!sync) {
			if (getvcc() < VCC_MIN) {
				for (i = 0; i < 5; i++) {
					LED_PORT ^= (1 << BATLOW_LED);
					_delay_ms(100);
				}
			}

			radio_pwr_dwn();
			sleep_enable();	
			sleep_bod_disable();
			sleep_cpu();
			sleep_disable();
			radio_listen();
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
	if (is_btn_pressed(PIND, LOCK_PIN)) {
		sync = 0;
		islock = 1;
	}
}

ISR(INT1_vect)
{
	if (is_btn_pressed(PIND, UNLOCK_PIN)) {
		sync = 0;
		isunlock = 1;
	}
}
