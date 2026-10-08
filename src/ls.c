#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "ls.h"

int compare_names(const void *a, const void *b)
{
    return strcmp(
        (const char *)a,
        (const char *)b);
}

void list_directory(const char *path, const Options *opt)
{
    DIR *dir = opendir(path);

    if (dir == NULL)
    {
        perror(path);
        return;
    }

    struct dirent *entry;

    char files[1000][256];
    int count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        /* -a */
        if (opt->show_all)
        {
            strcpy(files[count], entry->d_name);
            count++;
            continue;
        }

        /* -A */
        if (opt->almost_all)
        {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            strcpy(files[count], entry->d_name);
            count++;
            continue;
        }

        /* mặc định */
        if (entry->d_name[0] == '.')
        {
            continue;
        }

        strcpy(files[count], entry->d_name);
        count++;
    }

    closedir(dir);

    qsort(
        files,
        count,
        sizeof(files[0]),
        compare_names);

    for (int i = 0; i < count; i++)
    {
        printf("%s\n", files[i]);
    }
}