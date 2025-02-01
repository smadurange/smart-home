#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

struct radio_cfg {
	uint8_t netid;
	uint8_t nodeid;
	uint8_t payload_len;
};

void radio_init(const struct radio_cfg *cfg);

void radio_set_tx_power(int8_t val);

void radio_listen(void);

void radio_send(const char *data, uint8_t n);

uint8_t radio_recv(char *buf, uint8_t n);

#endif
