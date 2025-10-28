
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>

#include "packet.h"
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

void print_footer(stats_t* stats, struct in_addr* addr)
{
    printf(
        "--- %s ping statistics ---\n"
        "%li packets transmitted, %li packets received, %li%% packet loss\n",
        inet_ntoa(*addr),
        stats->send,
        stats->recv,
        stats->send == 0 ? 0 : ((stats->send - stats->recv) / stats->send * 100)
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
