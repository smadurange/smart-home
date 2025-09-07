#ifndef FPM_R503_H
#define FPM_R503_H

#include <stdint.h>

struct fpm_cfg {
	uint16_t status;
	uint16_t sysid;
	uint16_t cap;
	uint16_t sec_level;
	uint8_t addr[4];
	uint16_t pkt_size;
	uint16_t baud;
};

typedef enum {
	NIL = 0x00,
	RED = 0x01,
	BLUE = 0x02,
	PURPLE = 0x03
} LED_COLOR;

typedef enum {
	BREATHE = 0x01,
	FLASH = 0x02,
	ON = 0x03,
	OFF = 0x04,
	GRAD_ON = 0x05,
	GRAD_OFF = 0x06,
} LED_CTRL;

uint8_t fpm_init(void);

uint8_t fpm_get_cfg(struct fpm_cfg *cfg);

uint8_t fpm_set_pwd(uint32_t pwd);

uint16_t fpm_get_count(void);

uint8_t fpm_enroll(void);

uint16_t fpm_match(void);

void fpm_led(LED_CTRL ctrl, LED_COLOR color, uint8_t count);

#endif /* FPM_R50_H */
