#ifndef RFM69_H
#define RFM69_H

#include <stdint.h>

void rfm_init(uint8_t addr);

void rfm_sendto(uint8_t addr, uint8_t *data, uint8_t n);

#endif /* RFM69_H */
