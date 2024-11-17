#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/setbaud.h>

#define TEST_LED    PB1
#define LOCK_LED    PD6
#define UNLOCK_LED  PD7

#define SYN        0xA4
#define ADDR       0x44
#define LOCK_CMD   0x11
#define UNLOCK_CMD 0x22

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

static inline int is_btn_pressed(unsigned char btn)
{
	if (!((PIND >> btn) & 0x01)) {
		_delay_us(2000);
		return !((PIND >> btn) & 0x01);
	}
	
	return 0;
}

ISR(USART_RX_vect)
{
	unsigned char syn, addr, data, chk;

	syn  = usart_recv();
	addr = usart_recv();
	data = usart_recv();
	chk  = usart_recv();

	if(chk == (addr + data))
	{
		if(addr == ADDR)
		{
			if(data == LOCK_CMD) {
				PORTD |= (1 << LOCK_LED);
				PORTD &= ~(1 << UNLOCK_LED);
				
			} else if (data == UNLOCK_LED) {
				PORTD |= (1 << UNLOCK_LED);
				PORTD &= ~(1 << LOCK_LED);
			}
		}
		PORTB ^= (1 << TEST_LED);
	}
}
