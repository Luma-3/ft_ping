#ifndef FT__PING
#define FT__PING 1

#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

typedef struct s_param
{
    bool  verbose;
    char* addr;
} t_param;

void parse_arg(int ac, char** av, t_param* params);
int  resolve_host(char* addr_str, struct sockaddr_in* addr);

#endif
