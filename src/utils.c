
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <unistd.h>

#include "packet.h"
#include "ping.h"

double elapsed_time(struct s_time* time)
{
    long   sec_diff  = time->trecv.tv_sec - time->tsend.tv_sec;
    long   nsec_diff = time->trecv.tv_nsec - time->tsend.tv_nsec;
    double rtt_time  = sec_diff * 1000 + nsec_diff / 1e6;
    return rtt_time;
}

void pr_icmp(packet_t* packet, bool verbose)
{
    struct icmphdr* icmp = packet->icmphdr;

    switch (icmp->type)
    {
    case ICMP_UNREACH:
        printf("Destination Host Unreachable\n");
        break;
    case ICMP_TIME_EXCEEDED:
        printf("Time to live exceeded\n");
        break;
    case ICMP_REDIRECT:
        printf("Redirect (change route)\n");
        break;
    default:
        printf("ICMP type %d code %d\n", icmp->type, icmp->code);
        break;
    }
    if (verbose)
    {
        dump_packet(packet);
    }
}

void print_rep(
    t_ping*   ping,
    packet_t* packet,
    double    rtt_time,
    bool      verbose,
    bool      is_duplicate
)
{
    printf(
        "%li bytes from %s: ", packet->icmp_len, inet_ntoa(ping->addr.sin_addr)
    );
    if (packet->icmphdr->type == ICMP_ECHOREPLY)
    {
        printf(
            "icmp_seq=%i ttl=%i time=%.3f ms",
            ntohs(packet->icmphdr->un.echo.sequence),
            packet->iphdr->ttl,
            rtt_time
        );
    }
    else
    {
        pr_icmp(packet, verbose);
        return;
    }
    if (is_duplicate)
    {
        printf(" (DUP!)");
    }
    printf("\n");
}

void print_header(t_param* param, struct in_addr* addr, int payload_size)
{
    printf(
        "PING %s (%s) %i data bytes",
        param->addr,
        inet_ntoa(*addr),
        payload_size
    );
    if (param->optarg & OPT_VERBOSE)
    {
        int pid = getpid() & 0xFFFF;
        printf(", id 0x%x = %i", pid, pid);
    }
    printf("\n");
}

void print_footer(t_stats* stats, struct in_addr* addr)
{
    printf(
        "--- %s ping statistics ---\n"
        "%li packets transmitted, %li packets received,",
        inet_ntoa(*addr),
        stats->send,
        stats->recv
    );

    if (stats->dup != 0)
    {
        printf(" +%li duplicates,", stats->dup);
    }
    printf(
        " %li%% packet loss\n",
        stats->send == 0
            ? 0
            : (long)(((stats->send - stats->recv) / (double)stats->send) * 100)
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

struct timeval time_sub(struct timeval a, struct timeval b)
{
    struct timeval result;
    result.tv_sec  = a.tv_sec - b.tv_sec;
    result.tv_usec = a.tv_usec - b.tv_usec;
    if (result.tv_usec < 0)
    {
        result.tv_sec -= 1;
        result.tv_usec += 1000000;
    }
    return result;
}

struct timeval time_add(struct timeval a, struct timeval b)
{
    struct timeval result;
    result.tv_sec  = a.tv_sec + b.tv_sec;
    result.tv_usec = a.tv_usec + b.tv_usec;
    if (result.tv_usec >= 1000000)
    {
        result.tv_sec += 1;
        result.tv_usec -= 1000000;
    }
    return result;
}
