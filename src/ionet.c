#include "ionet.h"

#include <ionet.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

extern volatile int is_running;

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

void send_packet(
    int fd, struct sockaddr_in* addr, ssize_t seq, struct s_time* time
)
{
    uint8_t icmp_buff[sizeof(struct icmphdr) + PAYLOAD_SIZE];

    memset(icmp_buff, 0, sizeof(struct icmphdr) + PAYLOAD_SIZE);

    memset(&time->tsend, 0, sizeof(time->tsend));
    memset(&time->trecv, 0, sizeof(time->trecv));
    clock_gettime(CLOCK_MONOTONIC, &time->tsend);

    icmp_pack(seq, icmp_buff, (char*)&time->tsend);

    if (sendto(
            fd,
            icmp_buff,
            sizeof(struct icmphdr) + PAYLOAD_SIZE,
            0,
            (struct sockaddr*)addr,
            sizeof(*addr)
        ) < 0)
    {
        perror("sendto");
    }
}

enum recv_status
    recv_packet(int fd, struct s_time* time, packet_t* packet, uint8_t* buff)
{
    ssize_t nb_bytes = 0;
    ssize_t pack_len =
        sizeof(struct iphdr) + sizeof(struct icmphdr) + PAYLOAD_SIZE;

    memset(buff, 0, pack_len);
    memset(packet, 0, sizeof(*packet));

    nb_bytes = recvfrom(fd, buff, pack_len, 0, NULL, NULL);
    if (!is_running)
    {
        return STOP;
    }
    if (nb_bytes < 0)
    {
        perror("recv");
    }

    packet->pack_len = nb_bytes;
    packet->icmphdr  = icmp_unpack(buff, packet->pack_len, &packet->icmp_len);
    packet->iphdr    = ip_unpack(buff, &packet->ip_len);

    if (verif_its_me(packet) != true)
    {
        return CONTINUE;
    }

    clock_gettime(CLOCK_MONOTONIC, &time->trecv);
    if (verif_integrity(packet) != true)
    {
        printf("Packet Integrity KO");
    }
    return OK;
}
