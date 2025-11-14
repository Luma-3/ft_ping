#include <arpa/inet.h>
#include <bits/time.h>
#include <bits/types/struct_timeval.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "ionet.h"
#include "packet.h"
#include "ping.h"

volatile int g_is_running = 1;

static void handle_sigint(int sig)
{
    (void)sig;
    g_is_running = 0;
}

void refresh_stats(t_stats* stats, double elapsed_time, bool recv)
{
    if (recv)
        stats->recv++;
    stats->max = MAX(stats->max, elapsed_time);
    stats->min = MIN(stats->min, elapsed_time);

    double delta = elapsed_time - stats->avg;

    stats->avg += delta / stats->recv;
    stats->M2 += delta * (elapsed_time - stats->avg);
    stats->stddev =
        stats->recv - 1 != 0 ? sqrt(stats->M2 / (stats->recv - 1)) : 0;
}

bool handle_duplicate(t_ping* ping, struct icmphdr* icmp)
{
    uint16_t seq          = ntohs(icmp->un.echo.sequence);
    size_t   idx          = seq % PING_RECV_BUFF;
    bool     is_duplicate = ping->recv[idx] == 0;

    if (is_duplicate)
        ping->stats.dup++;

    ping->recv[idx] = 0;
    return is_duplicate;
}

void pr_packet(t_ping* ping, packet_t* packet, double elapsed, bool verbose)
{
    bool is_error = false;

    if (packet->icmphdr->type == ICMP_TIME_EXCEEDED ||
        packet->icmphdr->type == ICMP_UNREACH ||
        packet->icmphdr->type == ICMP_REDIRECT)
    {
        is_error = true;

        packet->inner_iphdr = ip_unpack(
            (uint8_t*)packet->icmphdr + sizeof(struct icmphdr),
            &packet->inner_ip_len
        );

        packet->inner_icmphdr = icmp_unpack(
            (uint8_t*)packet->inner_iphdr,
            packet->pack_len - sizeof(struct icmphdr) - packet->inner_ip_len,
            &packet->inner_icmp_len
        );
    }

    if (verif_its_me(packet) != true || verif_integrity(packet) != true)
    {
        return;
    }

    bool is_duplicate = handle_duplicate(
        ping, is_error ? packet->inner_icmphdr : packet->icmphdr
    );

    refresh_stats(&ping->stats, elapsed, is_error ? false : !is_duplicate);

    print_rep(ping, packet, elapsed, verbose, is_duplicate);
}

void loop(t_ping* ping, t_param* params)
{
    fd_set         read_fds;
    packet_t       packet;
    double         elapsed;
    struct s_time  time;
    struct timeval resp_time, now, last, start_time, intervl;

    intervl.tv_sec  = params->interval;
    intervl.tv_usec = 0;

    gettimeofday(&start_time, NULL);
    gettimeofday(&last, NULL);

    send_packet(ping, &time, params->size);

    while (g_is_running)
    {
        FD_ZERO(&read_fds);
        FD_SET(ping->sockfd, &read_fds);

        gettimeofday(&now, NULL);
        if (time_sub(now, last).tv_sec >= intervl.tv_sec)
        {
            gettimeofday(&last, NULL);
            send_packet(ping, &time, params->size);
        }

        resp_time = time_sub(time_add(last, intervl), now);

        int n = select(ping->sockfd + 1, &read_fds, NULL, NULL, &resp_time);
        if (n < 0)
        {
            perror("select");
            break;
        }

        if (params->optarg & OPT_TIMEOUT &&
            time_sub(now, start_time).tv_sec >= params->timeout)
        {
            break;
        }

        if (n == 1)
        {
            ssize_t ret = recv_packet(ping, &packet);
            if (ret < 0)
            {
                perror("recv_packet");
                break;
            }
            clock_gettime(CLOCK_MONOTONIC, &time.trecv);
            elapsed = elapsed_time(&time);
            pr_packet(ping, &packet, elapsed, params->optarg & OPT_VERBOSE);
        }
        if (params->optarg & OPT_COUNT &&
            ping->stats.recv >= (size_t)params->count)
        {
            g_is_running = 0;
        }
    }
}

void init_ping(t_ping* ping, t_param* params)
{
    memset(ping, 0, sizeof(*ping));

    ping->stats.min = __DBL_MAX__;

    ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (ping->sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    if (params->optarg & OPT_TTL)
    {
        printf("Setting TTL to %d\n", params->ttl);
        if (setsockopt(
                ping->sockfd,
                IPPROTO_IP,
                IP_TTL,
                &params->ttl,
                sizeof(params->ttl)
            ) < 0)
        {
            perror("sockopt");
            close(ping->sockfd);
            exit(1);
        }
    }

    if (resolve_host(params->addr, &ping->addr))
    {
        close(ping->sockfd);
        exit(1);
    }

    signal(SIGINT, handle_sigint);

    ping->buff_size =
        params->size + sizeof(struct icmphdr) + sizeof(struct iphdr);
    ping->buff = malloc(ping->buff_size);
    if (ping->buff == NULL)
    {
        perror("malloc");
        close(ping->sockfd);
        exit(1);
    }
}

int main(int ac, char** av)
{
    t_param params;
    t_ping  ping;

    memset(&params, 0, sizeof(params));

    if (getuid() != 0)
    {
        printf("root privilege is required\n");
        return 1;
    }

    parse_arg(ac, av, &params);

    init_ping(&ping, &params);
    print_header(&params, &ping.addr.sin_addr, params.size);
    loop(&ping, &params);
    print_footer(&ping.stats, &ping.addr.sin_addr);
    free(ping.buff);
    close(ping.sockfd);
    return 0;
}
