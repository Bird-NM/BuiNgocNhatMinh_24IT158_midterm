#include <string.h>

#include "options.h"

void init_options(Options *opt)
{
    opt->show_all = 0;
    opt->almost_all = 0;
}

void parse_options(int argc, char *argv[], Options *opt)
{
    init_options(opt);

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-a") == 0)
        {
            opt->show_all = 1;
        }
        if (strcmp(argv[i], "-A") == 0)
        {
            opt->almost_all = 1;
        }
    }
}