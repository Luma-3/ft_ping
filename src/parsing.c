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
        "    -i, --interval=NUMBER    interval between sending each packet (in "
        "seconds)\n"
        "    -c, --count=NUMBER       stop after sending (and receiving) "
        "NUMBER ECHO_REQUEST packets\n"
        "    -w, --timeout=NUMBER     time to wait for a response (in "
        "seconds)\n"
    );
}

static struct option longopt[] = {
    {"help", no_argument, NULL, '?'},
    {"verbose", no_argument, NULL, 'v'},
    {"interval", required_argument, NULL, 'i'},
    {"count", required_argument, NULL, 'c'},
    {"timeout", required_argument, NULL, 'w'},
    {"linger", required_argument, NULL, 'W'},
    {0, 0, 0, 0}
};

void parse_arg(int ac, char** av, t_param* params)
{
    int opt;

    while ((opt = getopt_long(ac, av, "?vi:c:w:W:", longopt, NULL)) != -1)
    {
        switch (opt)
        {
        case 'v':
            params->verbose = true;
            break;
        case '?':
            usage();
            exit(EXIT_SUCCESS);
        case 'i':
            g_argopt |= OPT_INTERVAL;
            g_intervl = atoi(optarg);
            if (g_intervl <= 0)
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
            g_argopt |= OPT_COUNT;
            g_count = atoi(optarg);
            if (g_count <= 0)
            {
                fprintf(
                    stderr, "ft_ping: usage error: Invalid count '%s'\n", optarg
                );
                exit(EXIT_FAILURE);
            }
            break;
        case 'w':
            g_argopt |= OPT_TIMEOUT;
            g_timeout = atoi(optarg);
            if (g_timeout <= 0)
            {
                fprintf(
                    stderr,
                    "ft_ping: usage error: Invalid timeout '%s'\n",
                    optarg
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
