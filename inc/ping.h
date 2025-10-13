#ifndef FT__PING
#define FT__PING 1

#include <netinet/in.h>
#include <stddef.h>

int resolve_host(char* addr_str, struct sockaddr_in* addr);

#endif
