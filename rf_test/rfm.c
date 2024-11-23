#include "rfm.h"
#include "spi.h"

#define READ_MASK  0x7F
#define WRITE_MASK 0x80

static inline void send_cmd(uint8_t addr, uint8_t val)
{
	uint8_t ra, wa;

	ra = addr | READ_MASK;
	wa = addr | WRITE_MASK; 

	while (spi_recv(ra) != val)
		spi_send(wa, val);
}

void rfm_init(uint8_t addr)
{
	spi_init();

	// mode: standby + packet
	send_cmd(0x01, 0x44);

	// rx interrupt on DPIO0
	send_cmd(0x25, 0x40);
	send_cmd(0x26, 0x07);

	// packet format: 8 bits + whitening + crc + addr filtering
	send_cmd(0x37, 0x52);
	send_cmd(0x38, 0x08);
	send_cmd(0x38, addr);

	// disable encryption
	send_cmd(0x3D, 0x02);
}

void rfm_send(uint8_t addr, uint8_t data)
{
	
}
