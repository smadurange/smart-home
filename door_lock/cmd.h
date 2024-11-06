#ifndef SA_CMD_H
#define SA_CMD_H

enum command {
	DOOR_LOCK,
	DOOR_UNLOCK
};

int cmd_cmp(const char *s, enum command c);

char * cmd_hash(enum command c);

#endif /* SA_CMD_H */
