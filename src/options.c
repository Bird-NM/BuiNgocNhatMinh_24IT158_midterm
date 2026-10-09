
#include <stdio.h>
#include <string.h>
#include "options.h"

void init_options(Options *opt)
{
    memset(opt, 0, sizeof(*opt));
}

int parse_options(int argc, char *argv[], Options *opt)
{
    init_options(opt);

    int end_options = 0;

    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];

        if (!end_options && strcmp(arg, "--") == 0)
        {
            end_options = 1;
            continue;
        }

        if (!end_options && arg[0] == '-' && arg[1] != '\0')
        {
            for (int j = 1; arg[j] != '\0'; j++)
            {
                switch (arg[j])
                {
                    case 'a': opt->show_all = 1; break;
                    case 'A': opt->almost_all = 1; break;
                    case 'c': opt->use_ctime = 1; break;
                    case 'd': opt->list_directory = 1; break;
                    case 'F': opt->classify = 1; break;
                    case 'f':
                        opt->unsorted = 1;
                        opt->show_all = 1;
                        break;
                    case 'h': opt->human_readable = 1; break;
                    case 'i': opt->show_inode = 1; break;
                    case 'k': opt->kilobytes = 1; break;
                    case 'l': opt->long_format = 1; break;
                    case 'n':
                        opt->long_format = 1;
                        opt->numeric_ids = 1;
                        break;
                    case 'q': opt->quote_nonprint = 1; break;
                    case 'r': opt->reverse = 1; break;
                    case 'R': opt->recursive = 1; break;
                    case 'S': opt->sort_size = 1; break;
                    case 's': opt->show_blocks = 1; break;
                    case 't': opt->sort_time = 1; break;
                    case 'u': opt->use_atime = 1; break;

                    default:
                        fprintf(stderr,
                                "myls: unknown option -- %c\n",
                                arg[j]);
                        return -1;
                }
            }
        }
        else
        {
            if (opt->path_count >= 256)
            {
                fprintf(stderr, "myls: too many paths\n");
                return -1;
            }

            opt->paths[opt->path_count++] = arg;
        }
    }

    return 0;
}