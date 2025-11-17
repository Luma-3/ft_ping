#include <stdint.h>
#ifndef FT__PING
#define FT__PING 1

#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>

#include "packet.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define PING_RECV_BUFF 1024

#define MAX_PAYLOAD_SIZE 21000 - sizeof(struct icmphdr) - sizeof(struct iphdr)
#define MAX_PACKET_SIZE  21000
#define PAYLOAD_SIZE     56 // Default payload size

typedef struct s_param
{
    int     optarg;
    char*   addr;
    int     timeout;
    size_t  size;
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

typedef struct s_stats
{
    size_t send;
    size_t recv;
    double min;
    double max;
    double avg;
    double M2;
    double stddev;
    long   dup;
} t_stats;

typedef struct s_ping
{
    int                sockfd;
    t_stats            stats;
    struct sockaddr_in addr;
    long               seq;
    int                recv[PING_RECV_BUFF];
    uint8_t*           buff;
    int                buff_size;
} t_ping;

void parse_arg(int ac, char** av, t_param* params);

double elapsed_time(struct s_time* time);

void print_rep(
    t_ping*   ping,
    packet_t* packet,
    double    rtt_time,
    bool      verbose,
    bool      is_duplicate
);

void print_header(t_param* param, struct in_addr* addr, int payload_size);
void print_footer(t_stats* stats, struct in_addr* addr);

struct timeval time_sub(struct timeval a, struct timeval b);
struct timeval time_add(struct timeval a, struct timeval b);
#endif
