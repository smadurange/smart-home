#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/setbaud.h>

#define LOCK_BTN    PD6
#define UNLOCK_BTN  PD7

#define SYN        0xA4
#define ADDR       0x01
#define LOCK_CMD   0x02
#define UNLOCK_CMD 0x03

static void usart_init(void)
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
#if USE_2X
	UCSR0A |= (1 << U2X0);
#else
	UCSR0A &= ~(1 << U2X0);
#endif
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void usart_send(unsigned char data)
{
	while (!(UCSR0A & (1 << UDRE0)))
		;
	UDR0 = data;
}

int main(void)
{
	usart_init();

	for (;;)
		;

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

static inline void lock(void)
{
	usart_send(SYN);
	usart_send(ADDR);
	usart_send(LOCK_CMD);
	usart_send(LOCK_CMD + ADDR);
}

static inline void unlock(void)
{
	usart_send(SYN);
	usart_send(ADDR);
	usart_send(UNLOCK_CMD);
	usart_send(UNLOCK_CMD + ADDR);
}

ISR(PCINT2_vect)
{
	if (is_btn_pressed(LOCK_BTN))
		lock();

	if (is_btn_pressed(UNLOCK_BTN))
		unlock();
}
