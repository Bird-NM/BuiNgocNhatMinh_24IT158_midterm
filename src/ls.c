#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "ls.h"

int compare_names(const void *a, const void *b)
{
    const FileInfo *fa = a;
    const FileInfo *fb = b;

    return strcmp(fa->name, fb->name);
}
int compare_time(const void *a, const void *b)
{
    const FileInfo *fa = a;
    const FileInfo *fb = b;

    if (fa->info.st_mtime < fb->info.st_mtime)
        return 1;

    if (fa->info.st_mtime > fb->info.st_mtime)
        return -1;

    return 0;
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

    FileInfo files[1000];
    int count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        /* -a */
        if (opt->show_all)
        {
            strcpy(files[count].name, entry->d_name);
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

            strcpy(files[count].name, entry->d_name);
            count++;
            continue;
        }

        /* mặc định */
        if (entry->d_name[0] == '.')
        {
            continue;
        }

        strcpy(files[count].name, entry->d_name);

        char fullpath[512];
        snprintf(fullpath,
                 sizeof(fullpath),
                 "%s/%s",
                 path,
                 entry->d_name);

        stat(fullpath, &files[count].info);

        count++;
    }

    closedir(dir);

    if (opt->sort_time)
    {
        qsort(
            files,
            count,
            sizeof(files[0]),
            compare_time);
    }
    else
    {
        qsort(
            files,
            count,
            sizeof(files[0]),
            compare_names);
    }
    if (opt->reverse)
    {
        for (int i = count - 1; i >= 0; i--)
        {
            printf("%s\n", files[i].name);
        }
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            printf("%s\n", files[i].name);
        }
    }
}