#include <arpa/inet.h>
#include <bits/time.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "ionet.h"
#include "packet.h"
#include "ping.h"

typedef struct stats_s
{
    size_t send;
    size_t recv;
    double min;
    double max;
    double avg;
    double M2;
    double stddev;
} stats_t;

void add_stat(stats_t* stats, bool recv, double elapsed_time)
{
    stats->send++;
    stats->recv += recv;

    if (!recv)
        return;
    stats->max = MAX(stats->max, elapsed_time);
    stats->min = MIN(stats->min, elapsed_time);

    double delta = elapsed_time - stats->avg;

    stats->avg += delta / stats->recv;
    stats->M2 += delta * (elapsed_time - stats->avg);
    stats->stddev = sqrt(stats->M2 / (stats->recv - 1));
}

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

void print_footer(stats_t* stats, struct in_addr* addr)
{
    printf(
        "--- %s ping statistics ---\n"
        "%li packets transmitted, %li packets received, %li%% packet loss\n",
        inet_ntoa(*addr),
        stats->send,
        stats->recv,
        ((stats->send - stats->recv) / stats->send * 100)
    );

    if (stats->recv == 0)
        return;
    printf(
        "round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
        stats->min,
        stats->avg,
        stats->max,
        stats->stddev
    );
}

void loop(int fd, struct sockaddr_in* addr)
{
    ssize_t       seq = 0;
    struct s_time time;
    bool          recv_pack = true;
    packet_t      packet;
    double        elapsed;
    uint8_t       buff[1024];
    stats_t       stats;

    int i = 0;

    memset(&stats, 0, sizeof(stats));
    stats.min = __DBL_MAX__;
    while (i++ < 6)
    {
        if (recv_pack == true)
        {
            send_packet(fd, addr, seq, &time);
            seq++;
            recv_pack = false;
        }

        enum recv_status ret = recv_packet(fd, &time, &packet, buff);
        if (ret == CONTINUE)
        {
            continue;
        }

        elapsed = elapsed_time(&time);
        print_rep(packet, addr, elapsed);
        add_stat(&stats, 1, elapsed);
        if (elapsed < 1000)
        {
            usleep((1000 - elapsed) * 1000);
        }
        recv_pack = true;
    }
    print_footer(&stats, &addr->sin_addr);
}

int main(int ac, char** av)
{
    t_param            params;
    struct sockaddr_in addr;
    memset(&params, 0, sizeof(params));
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
