
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <errno.h>

#include "ls.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef NAME_MAX
#define NAME_MAX 255
#endif


static const Options *current_options;

static time_t file_time(const FileInfo *f)
{
    if (current_options->use_atime)
        return f->info.st_atime;

    if (current_options->use_ctime)
        return f->info.st_ctime;

    return f->info.st_mtime;
}

static int compare_names(const void *a, const void *b)
{
    const FileInfo *fa = a;
    const FileInfo *fb = b;
    return strcmp(fa->name, fb->name);
}

static int compare_size(const void *a, const void *b)
{
    const FileInfo *fa = a;
    const FileInfo *fb = b;

    if (fa->info.st_size > fb->info.st_size) return -1;
    if (fa->info.st_size < fb->info.st_size) return 1;

    return compare_names(a, b);
}

static int compare_time(const void *a, const void *b)
{
    const FileInfo *fa = a;
    const FileInfo *fb = b;

    time_t ta = file_time(fa);
    time_t tb = file_time(fb);

    if (ta > tb) return -1;
    if (ta < tb) return 1;

    return compare_names(a, b);
}

static int is_hidden_name(const char *name)
{
    return name[0] == '.';
}

static int should_skip(const char *name, const Options *opt)
{
    if (opt->show_all || opt->unsorted)
        return 0;

    if (opt->almost_all)
        return strcmp(name, ".") == 0 ||
               strcmp(name, "..") == 0;

    return is_hidden_name(name);
}

static int make_path(char *dest, size_t size,
                     const char *dir, const char *name)
{
    int n;

    if (strcmp(dir, "/") == 0)
        n = snprintf(dest, size, "/%s", name);
    else
        n = snprintf(dest, size, "%s/%s", dir, name);

    return n >= 0 && (size_t)n < size;
}

static void permission_string(mode_t mode, char *out)
{
    out[0] = S_ISDIR(mode) ? 'd' :
             S_ISLNK(mode) ? 'l' :
             S_ISCHR(mode) ? 'c' :
             S_ISBLK(mode) ? 'b' :
             S_ISFIFO(mode) ? 'p' :
             S_ISSOCK(mode) ? 's' : '-';

    out[1]  = mode & S_IRUSR ? 'r' : '-';
    out[2]  = mode & S_IWUSR ? 'w' : '-';
    out[3]  = mode & S_IXUSR ? 'x' : '-';
    out[4]  = mode & S_IRGRP ? 'r' : '-';
    out[5]  = mode & S_IWGRP ? 'w' : '-';
    out[6]  = mode & S_IXGRP ? 'x' : '-';
    out[7]  = mode & S_IROTH ? 'r' : '-';
    out[8]  = mode & S_IWOTH ? 'w' : '-';
    out[9]  = mode & S_IXOTH ? 'x' : '-';

    if (mode & S_ISUID)
        out[3] = (mode & S_IXUSR) ? 's' : 'S';

    if (mode & S_ISGID)
        out[6] = (mode & S_IXGRP) ? 's' : 'S';

    if (mode & S_ISVTX)
        out[9] = (mode & S_IXOTH) ? 't' : 'T';

    out[10] = '\0';
}

static void print_size(off_t size, int human)
{
    if (!human)
    {
        printf("%" PRIdMAX, (intmax_t)size);
        return;
    }

    double value = (double)size;
    const char *units[] = {"B", "K", "M", "G", "T", "P"};
    int unit = 0;

    while (value >= 1024.0 && unit < 5)
    {
        value /= 1024.0;
        unit++;
    }

    if (unit == 0)
        printf("%.0f%s", value, units[unit]);
    else
        printf("%.1f%s", value, units[unit]);
}

static void print_blocks(const struct stat *info,
                         const Options *opt)
{
    long long blocks = (long long)info->st_blocks;

    if (opt->human_readable)
    {
        /* st_blocks tren UNIX duoc tinh theo don vi 512 byte */
        print_size((off_t)(blocks * 512), 1);
    }
    else if (opt->kilobytes)
    {
        long long kb = (blocks + 1) / 2;
        printf("%lld", kb);
    }
    else
    {
        printf("%lld", blocks);
    }
}

static void print_name(const char *name, const Options *opt)
{
    for (const unsigned char *p = (const unsigned char *)name;
         *p != '\0'; p++)
    {
        if (opt->quote_nonprint && (*p < 32 || *p == 127))
            putchar('?');
        else
            putchar(*p);
    }
}

static void print_suffix(const FileInfo *f, const Options *opt)
{
    if (!opt->classify)
        return;

    if (S_ISDIR(f->info.st_mode))
        putchar('/');
    else if (S_ISLNK(f->info.st_mode))
        putchar('@');
    else if (S_ISFIFO(f->info.st_mode))
        putchar('|');
    else if (S_ISSOCK(f->info.st_mode))
        putchar('=');
    else if (f->info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
        putchar('*');
}

static void print_long_entry(const FileInfo *f,
                             const Options *opt)
{
    char perms[11];
    char datebuf[64];
    char linkbuf[PATH_MAX];

    permission_string(f->info.st_mode, perms);

    struct passwd *pw = NULL;
    struct group *gr = NULL;

    if (!opt->numeric_ids)
    {
        pw = getpwuid(f->info.st_uid);
        gr = getgrgid(f->info.st_gid);
    }

    time_t timestamp;

    if (opt->use_atime)
        timestamp = f->info.st_atime;
    else if (opt->use_ctime)
        timestamp = f->info.st_ctime;
    else
        timestamp = f->info.st_mtime;

    struct tm *tm_info = localtime(&timestamp);

    if (tm_info)
        strftime(datebuf, sizeof(datebuf), "%b %e %H:%M", tm_info);
    else
        snprintf(datebuf, sizeof(datebuf), "??? ?? ??:??");

    printf("%s %2ju ", perms, (uintmax_t)f->info.st_nlink);

    if (opt->numeric_ids || !pw)
        printf("%-8ju ", (uintmax_t)f->info.st_uid);
    else
        printf("%-8s ", pw->pw_name);

    if (opt->numeric_ids || !gr)
        printf("%-8ju ", (uintmax_t)f->info.st_gid);
    else
        printf("%-8s ", gr->gr_name);

    print_size(f->info.st_size, opt->human_readable);
    printf(" %s ", datebuf);

    print_name(f->name, opt);

    if (opt->classify)
        print_suffix(f, opt);

    if (S_ISLNK(f->info.st_mode))
    {
        ssize_t n = readlink(f->path, linkbuf, sizeof(linkbuf) - 1);

        if (n >= 0)
        {
            linkbuf[n] = '\0';
            printf(" -> %s", linkbuf);
        }
    }

    putchar('\n');
}

static void print_entry(const FileInfo *f, const Options *opt)
{
    if (opt->show_inode)
        printf("%ju ", (uintmax_t)f->info.st_ino);

    if (opt->show_blocks)
    {
        print_blocks(&f->info, opt);
        putchar(' ');
    }

    if (opt->long_format)
    {
        print_long_entry(f, opt);
        return;
    }

    print_name(f->name, opt);
    print_suffix(f, opt);
    putchar('\n');
}

static int collect_entries(const char *path,
                           const Options *opt,
                           FileInfo **out,
                           size_t *out_count)
{
    DIR *dir = opendir(path);

    if (!dir)
    {
        perror(path);
        return -1;
    }

    size_t capacity = 100;
    size_t count = 0;

    FileInfo *files = malloc(capacity * sizeof(*files));

    if (!files)
    {
        perror("malloc");
        closedir(dir);
        return -1;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (should_skip(entry->d_name, opt))
            continue;

        if (count == capacity)
        {
            size_t new_capacity = capacity * 2;
            FileInfo *tmp = realloc(
                files, new_capacity * sizeof(*files));

            if (!tmp)
            {
                perror("realloc");
                free(files);
                closedir(dir);
                return -1;
            }

            files = tmp;
            capacity = new_capacity;
        }

        FileInfo *f = &files[count];

        snprintf(f->name, sizeof(f->name), "%s", entry->d_name);

        if (!make_path(f->path, sizeof(f->path), path, entry->d_name))
        {
            fprintf(stderr, "myls: path too long: %s/%s\n",
                    path, entry->d_name);
            continue;
        }

        if (lstat(f->path, &f->info) != 0)
        {
            perror(f->path);
            continue;
        }

        count++;
    }

    closedir(dir);

    if (!opt->unsorted)
    {
        if (opt->sort_size)
            qsort(files, count, sizeof(*files), compare_size);
        else if (opt->sort_time)
            qsort(files, count, sizeof(*files), compare_time);
        else
            qsort(files, count, sizeof(*files), compare_names);

        if (opt->reverse)
        {
            for (size_t i = 0; i < count / 2; i++)
            {
                FileInfo tmp = files[i];
                files[i] = files[count - 1 - i];
                files[count - 1 - i] = tmp;
            }
        }
    }

    *out = files;
    *out_count = count;
    return 0;
}

static void list_path(const char *path, const Options *opt,
                      int show_header, int recursive_call)
{
    struct stat info;

    if (lstat(path, &info) != 0)
    {
        perror(path);
        return;
    }

    /* -d: hien thi thu muc nhu mot file */
    if (!S_ISDIR(info.st_mode) || opt->list_directory)
    {
        FileInfo f;
        snprintf(f.name, sizeof(f.name), "%s", path);
        snprintf(f.path, sizeof(f.path), "%s", path);
        f.info = info;
        print_entry(&f, opt);
        return;
    }

    FileInfo *files = NULL;
    size_t count = 0;

    if (collect_entries(path, opt, &files, &count) != 0)
        return;

    if (show_header)
        printf("%s:\n", path);

    for (size_t i = 0; i < count; i++)
        print_entry(&files[i], opt);

    if (opt->recursive)
    {
        for (size_t i = 0; i < count; i++)
        {
            FileInfo *f = &files[i];

            if (!S_ISDIR(f->info.st_mode))
                continue;

            if (strcmp(f->name, ".") == 0 ||
                strcmp(f->name, "..") == 0)
                continue;

            putchar('\n');
            list_path(f->path, opt, 1, 1);
        }
    }

    free(files);
    (void)recursive_call;
}

void list_directory(const char *path, const Options *opt)
{
    current_options = opt;
    list_path(path, opt, 0, 0);
}