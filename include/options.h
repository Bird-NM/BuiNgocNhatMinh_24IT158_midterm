#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct
{
    int show_all;
    int almost_all; 
} Options;

void init_options(Options *opt);
void parse_options(int argc, char *argv[], Options *opt);

#endif