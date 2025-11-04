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
int          g_argopt     = 0;
int          g_intervl    = 1;
int          g_count      = -1;
int          g_timeout    = 5;
int          g_linger     = 5;

void add_recv(stats_t* stats, double elapsed_time)
{
    stats->recv++;

    stats->max = MAX(stats->max, elapsed_time);
    stats->min = MIN(stats->min, elapsed_time);

    double delta = elapsed_time - stats->avg;

    stats->avg += delta / stats->recv;
    stats->M2 += delta * (elapsed_time - stats->avg);
    stats->stddev = sqrt(stats->M2 / (stats->recv - 1));
}

void pr_packet(
    packet_t* packet, stats_t* stats, double elapsed, struct sockaddr_in* addr
)
{
    if (verif_its_me(packet) != true || verif_integrity(packet) != true)
    {
        return;
    }
    add_recv(stats, elapsed);
    if (g_argopt & OPT_COUNT && stats->recv >= (size_t)g_count)
    {
        g_is_running = 0;
    }
    print_rep(*packet, addr, elapsed);
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

void loop(int fd, struct sockaddr_in* addr)
{
    ssize_t        seq = 0;
    struct s_time  time;
    packet_t       packet;
    double         elapsed;
    uint8_t        buff[1024];
    stats_t        stats;
    fd_set         read_fds;
    struct timeval resp_time, now, last, start_time;

    struct timeval intervl = {.tv_sec = g_intervl, .tv_usec = 0};
    gettimeofday(&start_time, NULL);

    memset(&stats, 0, sizeof(stats));
    stats.min = __DBL_MAX__;

    send_packet(fd, addr, seq++, &time);
    stats.send++;

    gettimeofday(&last, NULL);

    while (g_is_running)
    {
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);

        gettimeofday(&now, NULL);
        if (time_sub(now, last).tv_sec >= intervl.tv_sec)
        {
            gettimeofday(&last, NULL);
            send_packet(fd, addr, seq++, &time);
            stats.send++;
        }

        resp_time = time_sub(time_add(last, intervl), now);

        int n = select(fd + 1, &read_fds, NULL, NULL, &resp_time);
        if (n < 0)
        {
            perror("select");
            break;
        }

        if (g_argopt & OPT_TIMEOUT &&
            time_sub(now, start_time).tv_sec >= g_timeout)
        {
            break;
        }

        if (n == 1)
        {
            ssize_t ret = recv_packet(fd, buff, &packet);
            if (ret < 0)
            {
                perror("recv_packet");
                break;
            }
            clock_gettime(CLOCK_MONOTONIC, &time.trecv);
            elapsed = elapsed_time(&time);
            pr_packet(&packet, &stats, elapsed, addr);
        }

        if (g_argopt & OPT_VERBOSE)
        {
            pr_icmp(&packet);
        }
    }
    print_footer(&stats, &addr->sin_addr);
}

void handle_sigint(int sig)
{
    (void)sig;
    g_is_running = 0;
}

int main(int ac, char** av)
{
    int                sock_fd;
    t_param            params;
    struct sockaddr_in addr;

    memset(&params, 0, sizeof(params));
    parse_arg(ac, av, &params);

    if (getuid() != 0)
    {
        printf("root privilege is required\n");
        return 1;
    }

    sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock_fd < 0)
    {
        perror("socket");
        return 1;
    }

    setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &(int){1}, sizeof(int));

    if (resolve_host(params.addr, &addr))
    {
        close(sock_fd);
        return 1;
    }

    print_header(params.addr, &addr.sin_addr);

    signal(SIGINT, handle_sigint);

    loop(sock_fd, &addr);
    return 0;
}
