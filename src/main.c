#include <stdio.h>
#include "ls.h"
#include "options.h"

int main(int argc, char *argv[])
{

    Options opt;

    parse_options(argc, argv, &opt);

    list_directory(".", &opt);

    return 0;
}