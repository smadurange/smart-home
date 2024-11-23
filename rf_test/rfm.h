#ifndef RFM69_H
#define RFM69_H

#include <stdint.h>

void rfm_init(void);

void rfm_send(uint8_t addr, uint8_t data);

#endif /* RFM69_H */
