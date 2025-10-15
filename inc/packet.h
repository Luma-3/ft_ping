#ifndef _PACKET_H
#define _PACKET_H 1

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stddef.h>
#include <sys/types.h>

#define PAYLOAD_SIZE 56

typedef struct packet_s
{
    struct iphdr*   iphdr;
    size_t          ip_len;
    struct icmphdr* icmphdr;
    size_t          icmp_len;
    size_t          pack_len;
} packet_t;

struct s_time
{
    struct timespec tsend;
    struct timespec trecv;
};

u_int16_t checksum(u_int16_t* ptr, size_t len);

void            icmp_pack(ssize_t seq, u_int8_t* buff, char* data);
struct icmphdr* icmp_unpack(void* buff, size_t buff_len, size_t* len);
struct iphdr*   ip_unpack(void* buff, size_t* len);

int verif_integrity(packet_t* packet);

int verif_its_me(packet_t* packet);

#endif // !_PACKET_H
