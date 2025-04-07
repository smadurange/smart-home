#include "util.h"

void xor(const char *k, const char *s, char *d, uint8_t n)
{
	int i;

	for (i = 0; i < n; i++)
		d[i] = s[i] ^ k[i];
}
