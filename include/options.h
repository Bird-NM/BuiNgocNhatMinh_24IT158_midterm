
#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct
{
    int show_all;       // -a
    int almost_all;     // -A
    int reverse;        // -r
    int sort_size;      // -S
    int show_blocks;    // -s
    int sort_time;      // -t
    int use_access_time;// -u
} Options;

void init_options(Options *opt);
void parse_options(int argc, char *argv[], Options *opt);

#endif