#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

u_int16_t checksum(u_int16_t* ptr, size_t len)
{
    u_int32_t sum = 0;

    while (len > 1)
    {
        sum += *ptr;
        ptr++;
        len -= 2;
    }

    if (len == 1)
    {
        sum += *(u_int8_t*)ptr;
    }

    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ((u_int16_t)~sum);
}

void icmp_pack(struct icmphdr* buff, ssize_t seq)
{
    buff->type             = ICMP_ECHO;
    buff->code             = 0;
    buff->checksum         = 0;
    buff->un.echo.id       = htons(getpid() & 0xFFFF);
    buff->un.echo.sequence = htons(seq);
}

struct icmphdr* icmp_unpack(void* buff, size_t buff_len, size_t* len)
{
    struct iphdr* ip_head = (struct iphdr*)buff;

    *len = buff_len - ip_head->ihl * 4;
    return (struct icmphdr*)(buff + (ip_head->ihl * 4));
}
