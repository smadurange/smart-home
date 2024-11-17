#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/setbaud.h>

#define LOCK_LED    PB1
#define LOCK_BTN    PD6
#define UNLOCK_BTN  PD7

#define SYN        0xA4
#define ADDR       0x44
#define LOCK_CMD   0x11
#define UNLOCK_CMD 0x22

static void usart_init(void)
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0B = (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void usart_send(unsigned char data)
{
	while (!(UCSR0A & (1 << UDRE0)))
		;
	UDR0 = data;
}

static inline void pcint2_init(void)
{
	PCICR |= (1 << PCIE2);
	PCMSK2 |= ((1 << PCINT22) | (1 << PCINT23));
}

static inline void lock(void)
{
	PORTB ^= (1 << LOCK_LED);

	usart_send(SYN);
	usart_send(ADDR);
	usart_send(LOCK_CMD);
	usart_send(LOCK_CMD + ADDR);
}

static inline void unlock(void)
{
	PORTB ^= (1 << LOCK_LED);

	usart_send(SYN);
	usart_send(ADDR);
	usart_send(UNLOCK_CMD);
	usart_send(UNLOCK_CMD + ADDR);
}

int main(void)
{
	DDRB |= (1 << LOCK_LED);
	PORTB |= (1 << LOCK_LED);

	usart_init();
	//pcint2_init();

	//sei();

	for (;;) {
		_delay_ms(4000);
		lock();
		_delay_ms(4000);
		unlock();
	}

	return 0;
}

static inline int is_btn_pressed(unsigned char btn)
{
	if (!((PIND >> btn) & 0x01)) {
		_delay_us(2000);
		return !((PIND >> btn) & 0x01);
	}
	
	return 0;
}

ISR(PCINT2_vect)
{
	if (is_btn_pressed(LOCK_BTN)) {
		PORTB ^= (1 << LOCK_LED);
		//lock();
	}

	if (is_btn_pressed(UNLOCK_BTN)) {
		PORTB ^= (1 << LOCK_LED);
		//unlock();
	}
}
