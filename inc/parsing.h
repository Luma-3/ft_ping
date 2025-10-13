#ifndef PARSING_H
#define PARSING_H 1

#include <getopt.h>
#include <stdbool.h>

typedef struct s_param
{
    bool  verbose;
    char* addr;
} t_param;

void parse_arg(int ac, char** av, t_param* params);

#endif // PARSING_H
