#include "parsing.h"

#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void usage()
{
    printf(
        "\nUsage\n    ft_ping [options] <destination>\n"
        "    Send ICMP ECHO_REQUEST packet to networks hosts.\n\n"
        "Arguments:\n"
        "    <destination>   DNS name or IP address\n\n"
        "Options:\n"
        "    -v              verbose output\n"
        "    -?              print help and exit\n"
    );
}

static struct option longopt[] = {
    {"help", no_argument, NULL, '?'},
    {"verbose", no_argument, NULL, 'v'},
    {0, 0, 0, 0}
};

void parse_arg(int ac, char** av, t_param* params)
{
    int opt;

    while ((opt = getopt_long(ac, av, "v?", longopt, NULL)) != -1)
    {
        switch (opt)
        {
        case 'v':
            params->verbose = true;
            break;
        case '?':
            usage();
            exit(EXIT_SUCCESS);
        }
    }

    if (optind < ac)
    {
        params->addr = av[optind];
        printf("Host: %s\n", params->addr);
        printf("Verbose mode: %s\n", params->verbose ? "ON" : "OFF");
    }
    else
    {
        fprintf(stderr, "ft_ping: usage error: Destination address required\n");
        usage();
        exit(EXIT_FAILURE);
    }
}
