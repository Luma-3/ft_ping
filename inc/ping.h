#ifndef FT__PING
#define FT__PING 1

#include <sys/types.h>
void icmp_pack(struct icmphdr* buff);

u_int16_t checksum(u_int16_t* ptr, size_t len);

#endif
