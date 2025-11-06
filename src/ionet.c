#include "ionet.h"

#include <asm-generic/errno.h>
#include <ionet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "packet.h"
#include "ping.h"

extern volatile int g_is_running;

void pr_icmp(packet_t* packet)
{
    struct icmphdr* icmp = packet->icmphdr;

    switch (icmp->type)
    {
    case ICMP_HOST_UNREACH:
        printf("Destination Host Unreachable\n");
        break;
    case ICMP_TIME_EXCEEDED:
        printf("Time to live exceeded\n");
        break;
    case ICMP_REDIRECT:
        printf("Redirect (change route)\n");
        break;
    default:
        break;
    }
}

void send_packet(t_ping* ping, struct s_time* time)
{

    uint8_t icmp_buff[sizeof(struct icmphdr) + PAYLOAD_SIZE];

    memset(icmp_buff, 0, sizeof(struct icmphdr) + PAYLOAD_SIZE);

    memset(&time->tsend, 0, sizeof(time->tsend));
    memset(&time->trecv, 0, sizeof(time->trecv));
    clock_gettime(CLOCK_MONOTONIC, &time->tsend);

    icmp_pack(ping->seq, icmp_buff, (char*)&time->tsend);

    if (sendto(
            ping->sockfd,
            icmp_buff,
            sizeof(struct icmphdr) + PAYLOAD_SIZE,
            0,
            (struct sockaddr*)&ping->addr,
            sizeof(ping->addr)
        ) < 0)
    {
        perror("sendto");
    }
    ping->stats.send++;
    ping->seq++;
}

ssize_t recv_packet(int fd, uint8_t* buff, packet_t* packet)
{
    ssize_t nb_bytes = 0;
    ssize_t pack_len =
        sizeof(struct iphdr) + sizeof(struct icmphdr) + PAYLOAD_SIZE;

    memset(buff, 0, pack_len);
    memset(packet, 0, sizeof(*packet));

    nb_bytes = recvfrom(fd, buff, pack_len, 0, NULL, NULL);

    if (nb_bytes >= 0)
    {
        packet->pack_len = nb_bytes;
        packet->icmphdr =
            icmp_unpack(buff, packet->pack_len, &packet->icmp_len);
        packet->iphdr = ip_unpack(buff, &packet->ip_len);
    }
    return nb_bytes;
}

int resolve_host(char* addr_str, struct sockaddr_in* addr)
{
    struct addrinfo hints, *result;
    int             err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_RAW;

    err = getaddrinfo(addr_str, NULL, &hints, &result);
    if (err != 0)
    {
        fprintf(stderr, "ping: %s: %s", addr_str, gai_strerror(err));
        return 1;
    }

    memcpy(addr, result->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(result);
    return 0;
}
