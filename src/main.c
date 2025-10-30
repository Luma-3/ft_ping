#include <arpa/inet.h>
#include <bits/time.h>
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
#include <sys/types.h>
#include <unistd.h>

#include "ionet.h"
#include "packet.h"
#include "ping.h"

volatile int is_running = 1;

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

void loop(int fd, struct sockaddr_in* addr, t_param* params)
{
    ssize_t       seq = 0;
    struct s_time time;
    bool          recv_pack = true;
    packet_t      packet;
    double        elapsed;
    uint8_t       buff[1024];
    stats_t       stats;

    memset(&stats, 0, sizeof(stats));
    stats.min = __DBL_MAX__;
    while (is_running)
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
        if (ret == STOP)
        {
            break;
        }

        if (params->verbose)
        {
            pr_icmp(&packet);
        }

        elapsed = elapsed_time(&time);
        if (ret == OK)
        {
            print_rep(packet, addr, elapsed);
        }
        add_stat(&stats, ret == OK, elapsed);

        if (elapsed < 1000)
        {
            usleep((1000 - elapsed) * 1000);
        }
        recv_pack = true;
    }
    print_footer(&stats, &addr->sin_addr);
}

void handle_sigint(int sig)
{
    (void)sig;
    is_running = 0;
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
    struct timeval timeout;
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
    {
        perror("setsockopt");
        close(fd);
        return 1;
    }

    if (resolve_host(params.addr, &addr))
    {
        close(fd);
        return 1;
    }

    print_header(params.addr, &addr.sin_addr);

    signal(SIGINT, handle_sigint);

    loop(fd, &addr, &params);
    return 0;
}
