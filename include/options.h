
#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct
{
    int show_all;         /* -a */
    int almost_all;       /* -A */
    int use_ctime;        /* -c */
    int list_directory;   /* -d */
    int classify;         /* -F */
    int unsorted;         /* -f */
    int human_readable;   /* -h */
    int show_inode;       /* -i */
    int kilobytes;        /* -k */
    int long_format;      /* -l */
    int numeric_ids;      /* -n */
    int quote_nonprint;   /* -q */
    int reverse;          /* -r */
    int recursive;        /* -R */
    int sort_size;        /* -S */
    int show_blocks;      /* -s */
    int sort_time;        /* -t */
    int use_atime;        /* -u */

    char *paths[256];
    int path_count;
} Options;

void init_options(Options *opt);
int parse_options(int argc, char *argv[], Options *opt);

#endif