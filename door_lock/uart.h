#ifndef UART_H
#define UART_H

void uart_init(void);

uint8_t uart_recv(void);

void uart_send(uint8_t c);

#endif /* UART_H */
