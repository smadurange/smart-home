#ifndef MY_UTIL_H
#define MY_UTIL_H

#include <stdint.h>

#define WDLEN  32

#define KEY       "dM>}jdb,6gsnC$J^K 8(I5vyPemPs%;K"
//#define SYN       "43iqr5$NB8SpN?Z/52{iVl>o|i!.'dsT"
//#define LOCK      "R,I7l^E4j]KyLR9'*Q{Jd'zu.~!84}Ij"
//#define UNLOCK    "R,I7l^E4j]KyLR9'*Q{Jd'zu.~!84}Ik"

#define SYN       "SYNSYNSYNSYNSYNSYNSYNSYNSYNSYNSY"
#define ACK       "ACKACKACKACKACKACKACKACKACKACKAC"
#define MSG       "MSGMSGMSGMSGMSGMSGMSGMSGMSGMSGMS"

void xor(const char *k, const char *s, char *d, uint8_t n);

#endif /* MY_UTIL_H */
