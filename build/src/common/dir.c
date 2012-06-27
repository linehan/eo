#define USE_ERRNO_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>
#include <langinfo.h>

#include "file.h"
#include "error.h"
#include "util.h"
#include "textutils.h"
#include "list.h"
#include "lib/bloom/bloom.h"


/****************************************************************************** 
 * DIRECTORY/FILE ITERATION 
 * 
 * Flexible iteration over the files in a directory, ensuring that logic 
 * can receive them in a serial fashion, and thus simplify its design
 * considerations.
 ******************************************************************************/

/**
 * filecount -- count the number of files in a directory
 * @dir   : directory
 * @filter: filters the files included in the total
 * Returns the number of files in 'dir' passing filter. DIR stream is rewound.
 */
int filecount(DIR *dir, int filter)
{
        struct dirent *dirp;
        struct stat dstat;
        int count=0;

        while ((dirp = readdir(dir)) != NULL) {
                /*
                 * If we cannot stat a file, we move on.
                 */
                if (stat(dirp->d_name, &dstat) == -1)
                        continue;
                /* 
                 * If the F_HID filter is not set, and the file
                 * is is hidden, i.e. preceded by a '.', move on.
                 */
                if (!hasvalue(filter, F_HID) && dirp->d_name[0] == '.')
                                continue;
                /* 
                 * If the file's type is included in the filter,
                 * increment the counter. 
                 */
                if ((hasvalue(filter, F_TYPE(dstat.st_mode))))
                                count++;
        }
        rewinddir(dir);
        return count;
}


/**
 * getfile -- Like strtok...
 * @dir   : directory
 * @filter: filters the files included in the total
 */
const char *getfile(DIR *dir, int filter)
{
        struct dirent *dirp;
        struct stat dstat;
        static DIR *_dir;

        if (dir != NULL)
                _dir = dir;

        while ((dirp = readdir(_dir)), dirp != NULL) {
                /*
                 * If we cannot stat a file, we move on.
                 */
                if (stat(dirp->d_name, &dstat) == -1)
                        continue;
                /* 
                 * If the F_HID filter is not set, and the file
                 * is is hidden, i.e. preceded by a '.', move on.
                 */
                if (dirp->d_name[0] == '.' && !hasvalue(filter, F_HID))
                        continue;
                /* 
                 * If the file's type is included in the filter,
                 * return that filename. 
                 */
                if (hasvalue(filter, F_TYPE(dstat.st_mode)))
                        return dirp->d_name;
        }
        rewinddir(_dir);
        return NULL;
}


/**
 * getdiff
 */
const char *getdiff(DIR *dir, int filter)
{
        static struct bloom_t *bloom;
        struct dirent *dirp;
        struct stat dstat;
        static DIR *_dir;

        if (dir != NULL) {
                if (!bloom)
                        bloom = bloom_new(250000, 3, fnv_hash, sdbm_hash, djb2_hash);

                _dir  = dir;
        }

        while ((dirp = readdir(_dir)), dirp != NULL) {
                /*
                 * If we cannot stat a file, we move on.
                 */
                if (stat(dirp->d_name, &dstat) == -1)
                        continue;
                /* 
                 * If the F_HID filter is not set, and the file
                 * is is hidden, i.e. preceded by a '.', move on.
                 */
                if (dirp->d_name[0] == '.' && !hasvalue(filter, F_HID))
                        continue;

                /*
                 * If the filename is in the Bloom filter, move on. 
                 */
                if (bloom_check(bloom, dirp->d_name))
                        continue;

                /* 
                 * If the file's type is included in the filter,
                 * add the filename to the Bloom filter and 
                 * return the filename to the caller.
                 */
                if (hasvalue(filter, F_TYPE(dstat.st_mode))) {
                        bloom_add(bloom, dirp->d_name);
                        return dirp->d_name;
                }
        }
        rewinddir(_dir);
        return NULL;
}




/****************************************************************************** 
 * DLISTs
 * 
 ******************************************************************************/


/* Node in a directory list */
struct dlist_node {
        char filename[PATHSIZE];
        struct list_node node;
};


/**
 * dlist_load -- Load the filenames in a directory into a dlist struct
 * @head  : head of a dlist
 * @dir   : directory to traverse
 * @filter: filters the files included in the list
 */
void dlist_load(struct list_head *head, DIR *dir, int filter)
{
        struct dirent *dirp;
        struct stat dstat;

        while ((dirp = readdir(dir)) != NULL) {
                /*
                 * If we cannot stat some file, we move on.
                 */
                if (stat(dirp->d_name, &dstat) == -1)
                        continue;
                /* 
                 * If the F_HID filter is not set, and the file
                 * is is hidden, i.e. preceded by a '.', move on.
                 */
                if (!hasvalue(filter, F_HID) && dirp->d_name[0] == '.')
                        continue;
                /* 
                 * If the file's type is included in the filter,
                 * add it to the list. 
                 */
                if ((hasvalue(filter, F_TYPE(dstat.st_mode)))) {
                        struct dlist_node *new;
                        new = calloc(1, sizeof(struct dlist_node));
                        strlcat(new->filename, dirp->d_name, PATHSIZE);
                        list_add(head, &new->node);
                }
        }
}


/**
 * dlist_load -- Delete the nodes in a dlist.
 * @head  : head of a dlist
 */
void dlist_del(struct list_head *head)
{
        struct dlist_node *tmp, *nxt;

        list_for_each_safe(head, tmp, nxt, node) {
                list_del(&tmp->node); 
                free(tmp);
        }
}


/**
 * dlist_print -- Print the nodes of a dlist.
 * @path   : head of a dlist
 * @options: the filters
 */
void dlist_print(const char *path, int options)
{
        LIST_HEAD(list);
        struct dlist_node *tmp;
        DIR *dir;

        dir = opendir(path);

        dlist_load(&list, dir, options);

        list_for_each(&list, tmp, node) {
                printf("%s\n", tmp->filename);
        }

        dlist_del(&list);
}


/**
 * dlist_diff -- Get the asdf.
 */
void dlist_diff(struct list_head *diff, struct list_head *old, struct list_head *new)
{
        struct dlist_node *old_tmp;
        struct dlist_node *new_tmp;

        list_for_each(new, new_tmp, node) {
                list_for_each(old, old_tmp, node) {
                        if (STRCMP(new_tmp->filename, old_tmp->filename))
                                continue;

                        struct dlist_node *new;
                        new = calloc(1, sizeof(struct dlist_node));
                        strlcat(new->filename, new_tmp->filename, PATHSIZE);
                        list_add(diff, &new->node);
                }
        }
}




/*void list_dir(DIR *dir, int options)*/
/*{*/
        /*struct stat   statbuf;*/
        /*struct dirent *ep;*/
        /*struct passwd *pwd;*/
        /*struct group  *grp;*/
        /*struct tm     *tm;*/
        /*char date[256];*/

        /*if (dir) {*/
                /*while ((ep = readdir(dir)) != NULL) {*/
                        /*if (stat(ep->d_name, &statbuf) == -1)*/
                                /*continue;*/

                        /*[> -- hidden files -- <]*/
                        /*if (!(options & F_HID)) {*/
                                /*if (ep->d_name[0] == '.')*/
                                        /*continue;*/
                        /*}*/

                        /*[> -- permissions -- <]*/
                        /*if (options & LPRM)*/
                                /*printf("%10.10s ", sperm(statbuf.st_mode));*/

                        /*[> -- user -- <]*/
                        /*if (options & LUSR) { */
                                /*if (pwd = getpwuid(statbuf.st_uid), pwd)*/
                                        /*printf("%.8s ", pwd->pw_name);*/
                                /*else*/
                                        /*printf("%d ", statbuf.st_uid);*/
                        /*}*/

                        /*[> -- group -- <] */
                        /*if (options & LGRP) {*/
                                /*if (grp = getgrgid(statbuf.st_gid), grp)*/
                                        /*printf("%-8.8s ", grp->gr_name);*/
                                /*else*/
                                        /*printf("%-8d ", statbuf.st_gid);*/
                        /*}*/

                        /*[> -- filesize -- <]*/
                        /*if (options & LSIZ)*/
                                /*printf("%9jd ", (intmax_t)statbuf.st_size);*/

                        /*[> -- date & time -- <]*/
                        /*if (options & LDAT) {*/
                                /*tm = localtime(&statbuf.st_mtime);*/
                                /*strftime(date, 256, nl_langinfo(D_T_FMT), tm);*/
                                /*printf("%s ", date);*/
                        /*}*/

                        /*[> -- filename -- <]*/
                        /*if (options & LNAM) {*/
                                /*printf("%s ", ep->d_name);*/
                        /*}*/

                        /*printf("\n");*/
                /*}*/
                /*rewinddir(dir);*/
        /*} else {*/
                /*bye("Couldn't open directory");*/
        /*}*/
/*}*/

