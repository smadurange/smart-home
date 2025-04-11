#ifndef MY_UTIL_H
#define MY_UTIL_H

#include <stdint.h>

#define KEY     "dM>}jdb,6gsnC$J^K 8(I5vyPemPs%;K"
#define LOCK    "R,I7l^E4j]KyLR9'*Q{Jd'zu.~!84}Ij"
#define UNLOCK  "R,I7l^E4j]KyLR9'*Q{Jd'zu.~!84}Ik"

#define WDLEN   32

void wdt_init(void);

void keygen(char *buf, uint8_t n);

void xor(const char *k, const char *s, char *d, uint8_t n);

#endif /* MY_UTIL_H */
