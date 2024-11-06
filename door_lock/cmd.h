#ifndef SA_CMD_H
#define SA_CMD_H

enum command {
	DOOR_LOCK,
	DOOR_UNLOCK
};

int is_valid_cmd(const char *s, enum command c);

char * get_cmd_hash(enum command c);

#endif /* SA_CMD_H */
