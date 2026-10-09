
#ifndef LS_H
#define LS_H

#include <sys/stat.h>
#include "options.h"
#include <limits.h>

typedef struct
{
char name[NAME_MAX + 1];
    char path[4096];
    struct stat info;
} FileInfo;

void list_directory(const char *path, const Options *opt);

#endif
