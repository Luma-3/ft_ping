#include <stdint.h>
#ifndef FT__PING
#define FT__PING 1

#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>

#include "packet.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

typedef struct s_param
{
    int     optarg;
    char*   addr;
    int     timeout;
    int     linger;
    int     interval;
    int     count;
    uint8_t ttl;
} t_param;

enum arg_opt
{
    OPT_TTL      = 1 << 0,
    OPT_INTERVAL = 1 << 1,
    OPT_COUNT    = 1 << 2,
    OPT_TIMEOUT  = 1 << 3,
    OPT_LINGER   = 1 << 4,
    OPT_VERBOSE  = 1 << 5,
};

typedef struct s_ping
{
    int                sockfd;
    struct sockaddr_in addr;
} t_ping;

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

void parse_arg(int ac, char** av, t_param* params);

double elapsed_time(struct s_time* time);
void   print_rep(packet_t packet, struct sockaddr_in* addr, double rtt_time);
void   print_header(char* param, struct in_addr* addr);
void   print_footer(stats_t* stats, struct in_addr* addr);
#endif
