#ifndef SA_SERIAL_H
#define SA_SERIAL_H

void serial_init(void);
void serial_write(unsigned char data);
void serial_write_line(const char *s);

#endif /* SA_SERIAL_H */
