#include "ionet.h"

#include <asm-generic/errno.h>
#include <ionet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
    uint8_t icmp_buff[sizeof(struct icmphdr) + MAX_PAYLOAD_SIZE];
    char*   data = malloc(payload_size);
    if (!data)
    {
        perror("malloc");
        return;
    }

    memset(icmp_buff, 0, sizeof(struct icmphdr) + MAX_PAYLOAD_SIZE);

    memset(&time->tsend, 0, sizeof(time->tsend));
    memset(&time->trecv, 0, sizeof(time->trecv));

    clock_gettime(CLOCK_MONOTONIC, &time->tsend);

    icmp_pack(ping->seq, icmp_buff, data, payload_size);
    free(data);

    if (sendto(
            ping->sockfd,
            icmp_buff,
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

ssize_t recv_packet(int fd, uint8_t* buff, packet_t* packet)
{
    ssize_t nb_bytes = 0;

    memset(buff, 0, MAX_PACKET_SIZE);
    memset(packet, 0, sizeof(*packet));

    nb_bytes = recvfrom(fd, buff, MAX_PACKET_SIZE, 0, NULL, NULL);

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
