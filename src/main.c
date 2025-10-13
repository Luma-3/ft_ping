#include <arpa/inet.h>
#include <bits/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "packet.h"
#include "parsing.h"
#include "ping.h"

void print_rep(
    packet_t            packet,
    struct sockaddr_in* addr,
    struct timespec*    tsend,
    struct timespec*    trecv
)
{

    long   sec_diff  = trecv->tv_sec - tsend->tv_sec;
    long   nsec_diff = trecv->tv_nsec - tsend->tv_nsec;
    double rtt_time  = sec_diff * 1000 + nsec_diff / 1e6;
    printf(
        "%li bytes from %s: icmp_seq=%i ttl=%i time=%.3f ms\n",
        packet.icmp_len + PAYLOAD_SIZE,
        inet_ntoa(addr->sin_addr),
        ntohs(packet.icmphdr->un.echo.sequence),
        packet.iphdr->ttl,
        rtt_time
    );
}

void print_header(char* param, struct in_addr* addr)
{
    printf(
        "PING %s (%s) %i data bytes\n", param, inet_ntoa(*addr), PAYLOAD_SIZE
    );
}

void loop(int fd, struct sockaddr_in* addr)
{
    struct icmphdr  icmphdr;
    ssize_t         seq = 0;
    u_int8_t        read_buff[400];
    ssize_t         nb_bytes = 0;
    struct timespec tsend, trecv;

    while (true)
    {
        sleep(1);

        memset(&icmphdr, 0, sizeof(icmphdr));
        icmp_pack(&icmphdr, seq++);
        icmphdr.checksum = checksum((u_int16_t*)&icmphdr, sizeof(icmphdr));

        memset(&tsend, 0, sizeof(tsend));
        memset(&trecv, 0, sizeof(trecv));

        clock_gettime(CLOCK_MONOTONIC, &tsend);
        if (sendto(
                fd,
                &icmphdr,
                sizeof(icmphdr),
                0,
                (struct sockaddr*)addr,
                sizeof(*addr)
            ) < 0)
        {
            perror("sendto");
        }
        memset(read_buff, 0, sizeof(read_buff));

        nb_bytes = recvfrom(fd, read_buff, sizeof(read_buff), 0, NULL, NULL);
        if (nb_bytes < 0)
        {
            perror("recv");
        }
        clock_gettime(CLOCK_MONOTONIC, &trecv);

        packet_t packet;
        memset(&packet, 0, sizeof(packet));
        packet.pack_len = nb_bytes;

        packet.icmphdr =
            icmp_unpack(read_buff, packet.pack_len, &packet.icmp_len);

        packet.iphdr = ip_unpack(read_buff, &packet.ip_len);
        if (verif_integrity(&packet) != true)
        {
            printf("Packet Integrity KO");
        }
        print_rep(packet, addr, &tsend, &trecv);
    }
}

int main(int ac, char** av)
{
    t_param            params;
    struct sockaddr_in addr;
    parse_arg(ac, av, &params);

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

    if (resolve_host(params.addr, &addr))
    {
        close(fd);
        return 1;
    }

    print_header(params.addr, &addr.sin_addr);

    loop(fd, &addr);
    return 0;
}
