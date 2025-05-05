#ifndef MY_UTIL_H
#define MY_UTIL_H

#include <stdint.h>

#define KEY     "dM>}jdb,6gsnC$J^K 8(I5vyPemPs%;K"
#define SYN     "dM>}jdb,6gsnC$J^K 8(I5vyPemPs%;O"
#define LOCK    "R,I7l^E4j]KyLR9'*Q{Jd'zu.~!84}Ij"
#define UNLOCK  "R,I7l^E4j]KyLR9'*Q{Jd'zu.~!84}IL"

#define WDLEN   32

int is_btn_pressed(uint8_t pin, uint8_t btn);

void xor(const char *k, const char *s, char *d, uint8_t n);

uint16_t getvcc(void);

void led_init(void);

void led_locked(void);

void led_unlocked(void);

void led_bat(void);

#endif /* MY_UTIL_H */
