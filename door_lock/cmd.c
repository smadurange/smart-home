#include <stdint.h>
#include <string.h>

#include "cmd.h"

#define XORLEN    32
#define KEY       "dM>}jdb,6gsnC$J^K 8(I5vyPemPs%;K"
#define ULOCK_CMD "43iqr5$NB8SpN?Z/52{iVl>o|i!.'dsT"

static char ulock_crypt_cmd[XORLEN];

static inline void xor(const char *s, char *d, uint8_t n)
{
	int i;

	for (i = 0; i < n && s[i]; i++)
		d[i] = s[i] ^ KEY[i];
}

int is_ulock_cmd(const char *s)
{
	char buf[XORLEN + 1];

	xor(s, buf, XORLEN);
	buf[XORLEN] = 0;
	return !strcmp(ULOCK_CMD, buf);
}

char * get_encrypted_ulock_cmd(void)
{
	xor(ULOCK_CMD, ulock_crypt_cmd, XORLEN);
	return ulock_crypt_cmd;
}
