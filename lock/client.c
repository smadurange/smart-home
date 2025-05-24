/* Lock front, connected to the fingerprint scanner */

#include <stdint.h>
#include <stdlib.h>

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "fpm.h"
#include "nrfm.h"
#include "util.h"

#define LOCK_PIN      PD3
#define UNLOCK_PIN    PD2
#define ENROLL_PIN    PD4

#define RX_IRQ_PIN    PC0
#define RX_DDR        DDRC
#define RX_PIN        PINC
#define RX_ICR        PCICR 
#define RX_IE         PCIE1
#define RX_INT        PCINT8
#define RX_MSK        PCMSK1
#define RX_INTVEC     PCINT1_vect

#define VCC_MIN       4000

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
	RX_DDR &= ~(1 << RX_IRQ_PIN);
	RX_ICR |= (1 << RX_IE);
	RX_MSK |= (1 << RX_INT);
}

static inline void init_btns(void)
{
	DDRD &= ~((1 << LOCK_PIN) | (1 << UNLOCK_PIN) | ENROLL_PIN);
	PORTD |= ((1 << LOCK_PIN) | (1 << UNLOCK_PIN) | ENROLL_PIN);

	EICRA = 0b00000000;
	EIMSK = (1 << INT0) | (1 << INT1);

	PCICR |= (1 << PCIE2);
	PCMSK2 |= (1 << PCINT20);
}

static inline void fpm_ok(void)
{
	fpm_led_on(BLUE);
	_delay_ms(500);
	fpm_led_off();
}

static inline void fpm_nok(void)
{
	fpm_led_on(RED);
	_delay_ms(1000);
	fpm_led_off();
}

int main(void)
{
	int i, retries;
	uint8_t rxaddr[ADDRLEN] = { 194, 178, 82 };
	uint8_t txaddr[ADDRLEN] = { 194, 178, 83 };

	char buf[WDLEN], key[WDLEN];

	wdt_off();
	led_init();

	fpm_init();
	if (fpm_get_count() == 0)
		fpm_enroll();

	init_rx();
	init_btns();

	radio_init(rxaddr);

	sei();
	radio_listen();

	for (;;) {
		if (!sync && (islock || isunlock)) {
			if (isunlock) {
				if (!fpm_match()) {
					isunlock = 0;
					fpm_nok();
					continue;
				}
				else
					fpm_ok();
			}

			xor(KEY, SYN, buf, WDLEN);
			retries = 0;
			do {
				sync = radio_sendto(txaddr, buf, WDLEN);
				retries++;
				_delay_ms(50);
			} while (!sync && retries < 40);
			
			if (!sync) {
				islock = 0;
				isunlock = 0;
			}
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
					if (radio_sendto(txaddr, buf, WDLEN))
						led_locked();
				} else if (isunlock) {
					isunlock = 0;
					xor(key, UNLOCK, buf, WDLEN);
					if (radio_sendto(txaddr, buf, WDLEN))
						led_unlocked();
				}
			}
		}

		if (!sync) {
			if (getvcc() < VCC_MIN) {
				for (i = 0; i < 5; i++) {
					led_bat();
					_delay_ms(100);
				}
			}

			radio_pwr_dwn();
			sleep_bod_disable();
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_mode();
			radio_listen();
		}
	}
	return 0;
}

ISR(RX_INTVEC)
{
	if (!(RX_PIN & (1 << RX_IRQ_PIN)))
		rxd = 1;
}

ISR(INT0_vect)
{
	sync = 0;
	isunlock = 1;
}

ISR(INT1_vect)
{
	if (is_btn_pressed(PIND, LOCK_PIN)) {
		sync = 0;
		islock = 1;
	}
}

ISR(PCINT2_vect)
{
	uint16_t id;

	if (is_btn_pressed(PIND, ENROLL_PIN)) {
		id = fpm_match();
		if (id == 1 || id == 2) {
			fpm_ok();
			_delay_ms(1000);
			if (fpm_enroll())
				fpm_ok();
		} else
			fpm_nok();
	}
}

