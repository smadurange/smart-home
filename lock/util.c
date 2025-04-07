#include "util.h"

void xor(const char *k, const char *s, char *d, int n)
{
	int i;

	for (i = 0; i < n; i++)
		d[i] = s[i] ^ k[i];
}
