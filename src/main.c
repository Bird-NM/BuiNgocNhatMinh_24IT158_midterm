
#include <stdio.h>
#include "options.h"
#include "ls.h"

int main(int argc, char *argv[])
{
    Options opt;

    if (parse_options(argc, argv, &opt) != 0)
        return 1;

    if (opt.path_count == 0)
    {
        list_directory(".", &opt);
        return 0;
    }

    for (int i = 0; i < opt.path_count; i++)
    {
        if (i > 0)
            putchar('\n');

        list_directory(opt.paths[i], &opt);
    }

    return 0;
}