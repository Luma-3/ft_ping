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
    bool  verbose;
    char* addr;
} t_param;

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
int  resolve_host(char* addr_str, struct sockaddr_in* addr);

double elapsed_time(struct s_time* time);
void   print_rep(packet_t packet, struct sockaddr_in* addr, double rtt_time);
void   print_header(char* param, struct in_addr* addr);
void   print_footer(stats_t* stats, struct in_addr* addr);
#endif
