#include <string.h>

#include "cmd.h"

#define XORLEN 32

#define KEY        "dM>}jdb,6gsnC$J^K 8(I5vyPemPs%;K"
#define LOCK_CMD   "43iqr5$NB8SpN?Z/52{iVl>o|i!.'dsT"
#define UNLOCK_CMD "R,I7l^E4j]KyLR9'*Q{Jd'zu.~!84}Ij"

static char cmd[XORLEN];

static inline void xor(const char *s, char *d, int n)
{
	int i;

	for (i = 0; i < n && s[i]; i++)
		d[i] = s[i] ^ KEY[i];
}

int cmd_cmp(const char *s, enum command c)
{
	int rc;
	char buf[XORLEN + 1];

	xor(s, buf, XORLEN);
	buf[XORLEN] = 0;

	switch (c) {
	case DOOR_LOCK:
		rc = strcmp(LOCK_CMD, buf) == 0;
		break;
	case DOOR_UNLOCK:
		rc = strcmp(UNLOCK_CMD, buf) == 0;
		break;
	default:
		rc = 0;
		break;
	}

	return rc;
}

char * cmd_hash(enum command c)
{
	switch (c) {
	case DOOR_LOCK:
		xor(LOCK_CMD, cmd, XORLEN);
		break;
	case DOOR_UNLOCK:
		xor(UNLOCK_CMD, cmd, XORLEN);
		break;
	default:
		cmd[0] = 0;
		break;
	} 

	return cmd;
}
