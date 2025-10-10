#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "ping.h"
typedef struct s_param
{
    bool  verbose;
    char* addr;
} t_param;

int parse_av(int ac, char** av, t_param* buff_p)
{
    if (ac != 2)
    {
        printf("Usage: %s <ip>\n", av[0]);
        return 1;
    }

    buff_p->addr = av[1];

    return 0;
}

void format_rep(struct icmphdr* icmp_rep, struct sockaddr_in* addr, size_t len)
{
    char buff_addr[128];
    memset(buff_addr, 0, sizeof(buff_addr));
    inet_ntop(AF_INET, (void*)addr, buff_addr, sizeof(buff_addr));
    printf("%li bytes from %s: icmp_seq=%i ttl=%i time=%f ms\n",
           len,
           buff_addr,
           icmp_rep->un.echo.sequence,
           0,
           0.0f);
}

void loop(int fd, struct sockaddr_in* addr)
{
    struct icmphdr packet;
    ssize_t        seq = 0;
    u_int8_t       read_buff[64];

    while (true)
    {
        sleep(1);
        memset(&packet, 0, sizeof(packet));
        icmp_pack(&packet, seq++);
        packet.checksum = checksum((u_int16_t*)&packet, sizeof(packet));

        if (sendto(fd, &packet, sizeof(packet), 0, (struct sockaddr*)addr, sizeof(*addr)) < 0)
        {
            perror("sendto");
        }
        memset(read_buff, 0, sizeof(read_buff));

        int nb = 0;
        nb     = recvfrom(fd, read_buff, sizeof(read_buff), 0, NULL, NULL);
        if (nb < 0)
        {
            perror("recv");
        }

        size_t          len;
        struct icmphdr* rep_packet    = icmp_unpack(read_buff, nb, &len);
        u_int16_t       recv_checksum = rep_packet->checksum;
        rep_packet->checksum          = 0;

        if (checksum((u_int16_t*)rep_packet, len) != recv_checksum)
        {
            printf("Packet integrity KO");
        }

        format_rep(rep_packet, addr, nb);
    }
}

int main(int ac, char** av)
{

    t_param params;
    if (parse_av(ac, av, &params))
        return 1;

    if (getuid() != 0)
    {
        printf("root privilege is required\n");
        return 1;
    }

    int fd;
    fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd < 0)
    {
        perror("socket");
        return 1;
    }

    // if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &(int){1}, sizeof(int))) {
    //   perror("setsockopt");
    //   close(fd);
    //   return 1;
    // }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = 0;
    inet_pton(AF_INET, params.addr, &addr.sin_addr);

    loop(fd, &addr);
    return 0;
}
