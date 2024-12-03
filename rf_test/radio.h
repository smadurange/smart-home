#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

struct radio_cfg {
	uint8_t payload_len;
};

void radio_init(struct radio_cfg *cfg);

void radio_send(const char *data, uint8_t n);

uint8_t radio_recv(char *buf, uint8_t n);

#endif
