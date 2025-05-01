#include <avr/io.h>
#include <util/setbaud.h>

#include "fpm.h"

static uint8_t start_code[] = { 0xEF, 0x01 };
static uint8_t addr[] = { 0xFF, 0xFF, 0xFF, 0xFF };

static inline void write(uint8_t c)
{
	while (!(UCSR0A & (1 << UDRE0)))
		;
	UDR0 = c;
}

static inline void write_bulk(uint8_t *data, uint16_t n)
{
	int i;

	for (i = 0; i < n; i++)
		write(data[i]);
}

static inline void send(uint8_t pktid, uint8_t *data, uint8_t n)
{
	int i;
	uint16_t pktlen, sum;

	pktlen = n + 2;

	write_bulk(start_code, 2);
	write_bulk(addr, 4);
	write(pktid);
	write((uint8_t)(pktlen >> 8));
	write((uint8_t)pktlen);

	sum = (pktlen >> 8) + (pktlen & 0xFF) + pktid;
	for (i = 0; i < n; i++) {
		write(data[i]);
		sum += data[i];
	}

	write((uint8_t)(sum >> 8));
	write((uint8_t)sum);
}

void fpm_init(void)
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

void fpm_print_config(void)
{
}

