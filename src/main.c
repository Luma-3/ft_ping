#include <arpa/inet.h>
#include <bits/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "packet.h"
#include "parsing.h"
#include "ping.h"

double elapsed_time(struct s_time* time)
{
    long   sec_diff  = time->trecv.tv_sec - time->tsend.tv_sec;
    long   nsec_diff = time->trecv.tv_nsec - time->tsend.tv_nsec;
    double rtt_time  = sec_diff * 1000 + nsec_diff / 1e6;
    return rtt_time;
}

void print_rep(packet_t packet, struct sockaddr_in* addr, double rtt_time)
{
    printf(
        "%li bytes from %s: icmp_seq=%i ttl=%i time=%.3f ms\n",
        packet.icmp_len,
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

enum recv_status
{
    CONTINUE,
    BREAK,
    OK
};

void send_packet(
    int fd, struct sockaddr_in* addr, ssize_t seq, struct s_time* time
)
{
    uint8_t icmp_buff[sizeof(struct icmphdr) + PAYLOAD_SIZE];

    memset(icmp_buff, 0, sizeof(icmp_buff));

    memset(&time->tsend, 0, sizeof(time->tsend));
    memset(&time->trecv, 0, sizeof(time->trecv));
    clock_gettime(CLOCK_MONOTONIC, &time->tsend);

    icmp_pack(seq, icmp_buff, (char*)&time->tsend);

    if (sendto(
            fd,
            icmp_buff,
            sizeof(icmp_buff),
            0,
            (struct sockaddr*)addr,
            sizeof(*addr)
        ) < 0)
    {
        perror("sendto");
    }
}

enum recv_status recv_packet(int fd, struct s_time* time, packet_t* packet)
{
    u_int8_t read_buff[400];
    ssize_t  nb_bytes = 0;

    memset(read_buff, 0, sizeof(read_buff));

    nb_bytes = recvfrom(fd, read_buff, sizeof(read_buff), 0, NULL, NULL);
    if (nb_bytes < 0)
    {
        perror("recv");
    }

    memset(packet, 0, sizeof(*packet));
    packet->pack_len = nb_bytes;

    packet->icmphdr =
        icmp_unpack(read_buff, packet->pack_len, &packet->icmp_len);
    packet->iphdr = ip_unpack(read_buff, &packet->ip_len);

    if (verif_its_me(packet) != true)
    {
        return CONTINUE;
    }
    clock_gettime(CLOCK_MONOTONIC, &time->tsend);
    if (verif_integrity(packet) != true)
    {
        printf("Packet Integrity KO");
    }
    return OK;
}

void loop(int fd, struct sockaddr_in* addr)
{
    ssize_t       seq = 0;
    struct s_time time;
    bool          recv_pack = true;
    packet_t      packet;
    double        elapsed;

    while (true)
    {
        if (recv_pack == true)
        {
            send_packet(fd, addr, seq, &time);
            seq++;
            recv_pack = false;
        }

        enum recv_status ret = recv_packet(fd, &time, &packet);
        if (ret == CONTINUE)
        {
            continue;
        }

        elapsed = elapsed_time(&time);
        print_rep(packet, addr, elapsed);
        if (elapsed < 1000)
        {
            usleep((1000 - elapsed) * 1000);
        }
        recv_pack = true;
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
