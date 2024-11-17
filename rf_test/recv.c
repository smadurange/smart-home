#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/setbaud.h>

#define TEST_LED    PB1
#define LOCK_LED    PD6
#define UNLOCK_LED  PD7

#define SYN    0xA1
#define FIN    0xB2
#define LOCK   0xC3
#define UNLOCK 0xD3

static void usart_init(void)
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0B = (1 << RXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static unsigned char usart_recv(void)
{
	while((UCSR0A & (1 << RXC0)) == 0);
	return UDR0;
}

static void led_init(void)
{
	DDRB |= (1 << TEST_LED);
	DDRD |= (1 << LOCK_LED) | (1 << UNLOCK_LED);
	PORTD |= (1 << LOCK_LED) | (1 << UNLOCK_LED);
}

int main(void)
{
	usart_init();
	led_init();

	sei();

	for (;;)
		;

	return 0;
}

ISR(USART_RX_vect)
{
	unsigned char data, buf;

	data = 0;
	buf = usart_recv();
	
	if (buf == SYN) {
		buf = usart_recv();

		while (buf != FIN) {
			buf = usart_recv();

			if (buf == SYN) {
				PORTB ^= (1 << TEST_LED);
				return;
			}

			if (buf != FIN)
				data = buf;
		}

		if(data == LOCK) {
			PORTD |= (1 << LOCK_LED);
			PORTD &= ~(1 << UNLOCK_LED);
		} else if (data == UNLOCK) {
			PORTD |= (1 << UNLOCK_LED);
			PORTD &= ~(1 << LOCK_LED);
		}
	}

}
