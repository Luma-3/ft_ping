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

void send_packet(t_ping* ping, struct s_time* time, int payload_size)
{

    memset(&time->tsend, 0, sizeof(time->tsend));
    memset(&time->trecv, 0, sizeof(time->trecv));

    clock_gettime(CLOCK_MONOTONIC, &time->tsend);

    icmp_pack(ping->seq, ping->buff, payload_size);

    if (sendto(
            ping->sockfd,
            ping->buff,
            sizeof(struct icmphdr) + payload_size,
            0,
            (struct sockaddr*)&ping->addr,
            sizeof(ping->addr)
        ) < 0)
    {
        perror("sendto");
    }
    ping->stats.send++;
    ping->recv[ping->seq % PING_RECV_BUFF] = 1;
    ping->seq++;
}

ssize_t recv_packet(t_ping* ping, packet_t* packet)
{
    ssize_t nb_bytes = 0;

    memset(ping->buff, 0, ping->buff_size);
    memset(packet, 0, sizeof(*packet));

    nb_bytes =
        recvfrom(ping->sockfd, ping->buff, ping->buff_size, 0, NULL, NULL);

    if (nb_bytes >= 0)
    {
        packet->pack_len = nb_bytes;
        packet->icmphdr =
            icmp_unpack(ping->buff, packet->pack_len, &packet->icmp_len);
        packet->iphdr = ip_unpack(ping->buff, &packet->ip_len);
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
        fprintf(stderr, "ping: %s: %s\n", addr_str, gai_strerror(err));
        return 1;
    }

    memcpy(addr, result->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(result);
    return 0;
}
