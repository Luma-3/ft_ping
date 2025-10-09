#include <arpa/inet.h>
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

    struct icmphdr packet;
    icmp_pack(&packet);
    packet.checksum = checksum((u_int16_t*)&packet, sizeof(packet));

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

    int n = sendto(fd, &packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(addr));
    printf("n: %d\n", n);

    u_int8_t buff[1000];
    memset(buff, 0, sizeof(buff));

    int nb = 0;
    nb     = recvfrom(fd, buff, sizeof(buff), 0, NULL, NULL);
    if (nb < 0)
    {
        perror("recv");
    }

    printf("buff:%i | %s", nb, buff);
    return 0;
}
