#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

struct radio_cfg {
	uint8_t node_id;
	uint8_t network_id;
	uint8_t payload_len;
};

void radio_init(struct radio_cfg *cfg);

void radio_sendto(uint8_t addr, const char *data, uint8_t n);

uint8_t radio_recv(char *buf, uint8_t n);

#endif
