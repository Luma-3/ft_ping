
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int resolve_host(char* addr_str, struct sockaddr_in* addr)
{
    struct addrinfo hints, *result;
    int             err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_RAW;

    err = getaddrinfo(addr_str, NULL, &hints, &result);
    if (err != 0)
    {
        fprintf(stderr, "ping: %s: %s", addr_str, gai_strerror(err));
        return 1;
    }

    memcpy(addr, result->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(result);
    return 0;
}
