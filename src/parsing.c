#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ping.h"

void usage()
{
    printf(
        "\nUsage\n    ft_ping [options] <destination>\n"
        "    Send ICMP ECHO_REQUEST packet to networks hosts.\n\n"
        "Arguments:\n"
        "    <destination>            DNS name or IP address\n\n"
        "Options:\n"
        "    -v, --verbose            verbose output\n"
        "    -?, --help               print help and exit\n"
        "    -i, --interval=N         wait N secondes between sending each "
        "packet\n"
        "    -c, --count=N            stop after sending N packets\n"
        "    -w, --timeout=N          stop after N seconds\n"
        "    -W, --linger=N           N secondes to wait for response\n"
        "    -t, --ttl=N              specify N as time-to-live\n"
    );
}

static struct option longopt[] = {
    {"help", no_argument, NULL, '?'},
    {"verbose", no_argument, NULL, 'v'},
    {"interval", required_argument, NULL, 'i'},
    {"count", required_argument, NULL, 'c'},
    {"timeout", required_argument, NULL, 'w'},
    {"linger", required_argument, NULL, 'W'},
    {"ttl", required_argument, NULL, 't'},
    {0, 0, 0, 0}
};

void parse_arg(int ac, char** av, t_param* params)
{
    int opt;

    params->count    = -1;
    params->interval = 1;
    params->timeout  = 5;
    params->linger   = 5;

    while ((opt = getopt_long(ac, av, "?vi:c:w:W:t:", longopt, NULL)) != -1)
    {
        switch (opt)
        {
        case 'v':
            params->optarg |= OPT_VERBOSE;
            break;
        case '?':
            usage();
            exit(EXIT_SUCCESS);
        case 'i':
            params->optarg |= OPT_INTERVAL;
            params->interval = atoi(optarg);
            if (params->interval <= 0)
            {
                fprintf(
                    stderr,
                    "ft_ping: usage error: Invalid interval '%s'\n",
                    optarg
                );
                exit(EXIT_FAILURE);
            }
            break;
        case 'c':
            params->optarg |= OPT_COUNT;
            params->count = atoi(optarg);
            if (params->count <= 0)
            {
                fprintf(
                    stderr, "ft_ping: usage error: Invalid count '%s'\n", optarg
                );
                exit(EXIT_FAILURE);
            }
            break;
        case 'w':
            params->optarg |= OPT_TIMEOUT;
            params->timeout = atoi(optarg);
            if (params->timeout <= 0)
            {
                fprintf(
                    stderr,
                    "ft_ping: usage error: Invalid timeout '%s'\n",
                    optarg
                );
                exit(EXIT_FAILURE);
            }
            break;
        case 'W':
            params->optarg |= OPT_LINGER;
            params->linger = atoi(optarg);
            if (params->linger <= 0)
            {
                fprintf(
                    stderr,
                    "ft_ping: usage error: Invalid linger '%s'\n",
                    optarg
                );
                exit(EXIT_FAILURE);
            }
            break;
        case 't':
            params->optarg |= OPT_TTL;
            params->ttl = atoi(optarg);
            if (params->ttl == 0)
            {
                fprintf(
                    stderr, "ft_ping: usage error: Invalid ttl '%s'\n", optarg
                );
                exit(EXIT_FAILURE);
            }
            break;
        default:
            fprintf(
                stderr, "ft_ping: usage error: Unknown option '-%c'\n", optopt
            );
            usage();
        }
    }

    if (optind < ac)
    {
        params->addr = av[optind];
    }
    else
    {
        fprintf(stderr, "ft_ping: usage error: Destination address required\n");
        usage();
        exit(EXIT_FAILURE);
    }
}
