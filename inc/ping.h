#include <stddef.h>
#ifndef FT__PING
#define FT__PING 1

#include <netinet/ip_icmp.h>
#include <sys/types.h>

u_int16_t checksum(u_int16_t* ptr, size_t len);

void            icmp_pack(struct icmphdr* buff, ssize_t seq);
struct icmphdr* icmp_unpack(void* buff, size_t buff_len, size_t* len);

#endif
