
#ifndef LS_H
#define LS_H

#include <sys/stat.h>
#include "options.h"

typedef struct
{
    char name[256];
    char path[4096];
    struct stat info;
} FileInfo;

void list_directory(const char *path, const Options *opt);

#endif
