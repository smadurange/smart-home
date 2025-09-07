#include <stdint.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "fpm.h"
#include "uart.h"

#define BAT_MIN    5100

#define SRVO_PIN   PB1
#define SRVO_DDR   DDRB

#define PWM_MIN    500
#define PWM_MID    1600
#define PWM_MAX    2550
#define PWM_TOP    19999

#define LED_FPM_PIN   PD5
#define LED_BACK_PIN  PD6
#define LED_DDR       DDRD
#define LED_PORT      PORTD

#define PWR_BAT    PB2
#define PWR_SRVO   PB3
#define PWR_FPM    PB4
#define PWR_DDR    DDRB
#define PWR_PORT   PORTB

#define FPM_UNLOCK_PIN    PC1    /*back up for FPM's touch sensor */
#define FPM_LOCK_PIN      PC2
#define ENROLL_PIN        PC3
#define BACK_LOCK_PIN     PC4
#define BACK_UNLOCK_PIN   PC5
#define INPUT_DDR         DDRC
#define INPUT_PORT        PORTC

#define FPM_UNLOCK_INT    PCINT9
#define FPM_LOCK_INT      PCINT10
#define ENROLL_INT        PCINT11
#define BACK_LOCK_INT     PCINT12
#define BACK_UNLOCK_INT   PCINT13
#define INPUT_INT         PCIE1
#define INPUT_INT_MSK     PCMSK1
#define INPUT_INT_VEC     PCINT1_vect

enum CTRL { 
	NONE = 0, 
	LOCK_FPM = 1, 
	LOCK_BACK = 2, 
	UNLOCK_FPM = 3, 
	UNLOCK_BACK = 4,
	UNLOCK_FPM_2 = 5, 
	ENROLL = 6 
}; 

static volatile enum CTRL cmd = NONE;

static inline void pwron_bat(void)
{
	PWR_PORT &= ~(1 << PWR_BAT);
}

static inline void pwroff_bat(void)
{
	PWR_PORT |= (1 << PWR_BAT);
}

static inline void pwron_fpm(void)
{
	PWR_PORT &= ~(1 << PWR_FPM);
	_delay_ms(50);
}

static inline void pwroff_fpm(void)
{
	PWR_PORT |= (1 << PWR_FPM);
}

static inline void pwron_srvo(void)
{
	PWR_PORT |= (1 << PWR_SRVO);
}

static inline void pwroff_srvo(void)
{
	PWR_PORT &= ~(1 << PWR_SRVO);
}

static inline void lock(void)
{
	pwron_srvo();
	OCR1A = PWM_MID;
	_delay_ms(500);
	pwroff_srvo();
}

static inline void unlock(void)
{
	pwron_srvo();
	OCR1A = PWM_MAX;
	_delay_ms(500);
	pwroff_srvo();
}

static inline void flash_led(void)
{
	TCCR0A = (1 << COM0A0) | (1 << COM0B0) | (1 << WGM01);
	OCR0A = 255;
	OCR0B = 255;
	TCCR0B = (1 << CS02) | (1 << CS00);
}

static inline void stop_led(void)
{
	TCCR0B = 0;
	TCCR0A = 0;
	LED_PORT &= ~((1 << LED_FPM_PIN) | (1 << LED_BACK_PIN));
}

static void check_bat(void)
{
	uint16_t vbg, vcc;

	pwron_bat();

	ADMUX |= (1 << REFS1) | (1 << REFS0);
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); /* clk: 50-200 kHz */

	_delay_us(500); /* https://www.sciencetronics.com/greenphotons/?p=1521 */

	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC))
		; 

	ADCSRA &= ~(1 << ADEN);
	vbg = (1100UL * ADC) / 1024;
	ADCSRA &= ~(1 << ADEN);
	
	pwroff_bat();

	vcc = (vbg * 66) / 10; /* 56k/10k divider */
	if (vcc < BAT_MIN)
		flash_led();
}

int main(void)
{
	uint16_t id;

	/* disable wdt */
	cli();
	wdt_reset();
	MCUSR &= ~(1 << WDRF);
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = 0x00;

	PWR_DDR |= (1 << PWR_BAT) | (1 << PWR_FPM) | (1 << PWR_SRVO);
	pwroff_bat();

	uart_init();
	pwron_fpm();
	fpm_init();

	/* servo */
	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);
	TCCR1B |= (1 << CS11);
	ICR1 = PWM_TOP;
	TCCR1A |= (1 << COM1A1);
	SRVO_DDR |= (1 << SRVO_PIN);

	/* bat check */
	LED_DDR |= (1 << LED_FPM_PIN) | (1 << LED_BACK_PIN);
	LED_PORT &= ~((1 << LED_FPM_PIN) | (1 << LED_BACK_PIN));

	DDRD &= ~(1 << PD2); /* FPM unlock pin */
	PORTD |= (1 << PD2); /* FPM unlock pin internal pull-up */
	EICRA = 0b00000000;  
	EIMSK = (1 << INT0); /* FPM unlock interrupt */

	INPUT_DDR &= ~((1 << FPM_LOCK_PIN) | (1 << FPM_UNLOCK_PIN) | 
                   (1 << BACK_LOCK_PIN) | (1 << BACK_UNLOCK_PIN) |
	               (1 << ENROLL_PIN));

	INPUT_PORT |= ((1 << FPM_LOCK_PIN) | (1 << FPM_UNLOCK_PIN) | 
                   (1 << BACK_LOCK_PIN) | (1 << BACK_UNLOCK_PIN) | 
	               (1 << ENROLL_PIN));

	PCICR |= (1 << INPUT_INT);
	INPUT_INT_MSK |= ((1 << FPM_LOCK_INT) | (1 << FPM_UNLOCK_INT) | 
	                  (1 << BACK_LOCK_INT) | (1 << BACK_UNLOCK_INT) |
                      (1 << ENROLL_INT));

	for (;;) {
		check_bat();

		switch(cmd) {
		case LOCK_FPM:
			lock();
			fpm_led(FLASH, RED, 1);
			break;
		case LOCK_BACK:
			lock();
			break;
		case UNLOCK_FPM:
		case UNLOCK_FPM_2:
			if (fpm_match()) {
				fpm_led(BREATHE, BLUE, 1);
				unlock();
			} else {
				fpm_led(BREATHE, RED, 1);
			}
			break;
		case UNLOCK_BACK:
			unlock();
			fpm_led(FLASH, BLUE, 1);
			break;
		case ENROLL:
			id = fpm_match();
			if (id == 1 || id == 2) {
				fpm_led(BREATHE, BLUE, 1);
				_delay_ms(1000);
				if (fpm_enroll())
					fpm_led(BREATHE, BLUE, 1);
				else
					fpm_led(BREATHE, RED, 1);
			}
			break;
		default:
			break;
		}

		cmd = NONE;
		_delay_ms(500);

		pwroff_fpm();
		stop_led();

		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_enable();
		sleep_bod_disable();
		sei();
		sleep_cpu();

		cli();
		sleep_disable();
		pwron_fpm();
		fpm_init();
	}
	return 0;
}

static inline int is_pressed(uint8_t btn)
{
	if (!((PINC >> btn) & 0x01)) {
		_delay_ms(50);
		return !((PINC >> btn) & 0x01);
	}
	return 0;
}

ISR(INT0_vect)
{
	cmd = UNLOCK_FPM;
}

ISR(INPUT_INT_VEC)
{
	cli();

	if (is_pressed(FPM_LOCK_PIN))
		cmd = LOCK_FPM;
	else if (is_pressed(FPM_UNLOCK_PIN))
		cmd = UNLOCK_FPM_2;
	else if (is_pressed(BACK_LOCK_PIN))
		cmd = LOCK_BACK;
	else if (is_pressed(BACK_UNLOCK_PIN))
		cmd = UNLOCK_BACK;
	else if (is_pressed(ENROLL_PIN))
		cmd = ENROLL;
	else
		cmd = NONE;

	sei();
}
