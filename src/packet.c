#include <netinet/in.h>
#include <netinet/ip_icmp.h>
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

void icmp_pack(struct icmphdr* buff)
{
    buff->type             = ICMP_ECHO;
    buff->code             = 0;
    buff->checksum         = 0;
    buff->un.echo.id       = htons(getpid() & 0xFFFF);
    buff->un.echo.sequence = htons(0);
}
