#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/setbaud.h>

#define LOCK_LED    PB1
#define LOCK_BTN    PD6
#define UNLOCK_BTN  PD7

#define SYN    0xA1
#define FIN    0xB2
#define LOCK   0xC3
#define UNLOCK 0xD3

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
	PORTB |= (1 << LOCK_LED);

	usart_send(SYN);
	usart_send(LOCK);
	usart_send(FIN);
}

static inline void unlock(void)
{
	PORTB &= ~(1 << LOCK_LED);

	usart_send(SYN);
	usart_send(UNLOCK);
	usart_send(FIN);
}

int main(void)
{
	DDRB |= (1 << LOCK_LED);
	PORTB |= (1 << LOCK_LED);

	usart_init();

	for (;;) {
		_delay_ms(4000);
		lock();
		_delay_ms(4000);
		unlock();
	}

	return 0;
}

