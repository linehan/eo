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
#include "list/list.h"
#include "lib/bloom/bloom.h"
#include "bits.h"
#include "dir.h"


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
                if (dirp->d_name[0] == '.')
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
                if (dirp->d_name[0] == '.')
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
const char *_getdiff(DIR *dir, int filter)
{
        static struct bloom_t *bloom;
        struct dirent *dirp;
        struct stat dstat;
        static DIR *_dir;

        if (dir != NULL) {
                _dir = dir;
                if (!bloom)
                        bloom = bloom_new(250000, 3, fnv_hash, sdbm_hash, djb2_hash);
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
                if (dirp->d_name[0] == '.')
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

const char *getdiff(DIR *dir, int filter)
{
        static bool running = false;
        const char *filename;
        DIR *_dir = NULL; 

        if (!running) {
                _dir    = dir;
                running = true;
        }

        filename = _getdiff(_dir, filter);

        if (filename == NULL)
                running = false;

        return filename;
}



void nav_mark(struct nav_t *breadcrumb)
{
        strlcpy(breadcrumb->home, scwd(), PATHSIZE); 
}

void nav_shift(struct nav_t *breadcrumb, const char *path)
{
        nav_mark(breadcrumb);
        chdir(path);
        breadcrumb->away = true;
}

void nav_revert(struct nav_t *breadcrumb)
{
        if (breadcrumb->away) {
                chdir(breadcrumb->home);
                breadcrumb->away = false;
        }
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
                if (dirp->d_name[0] == '.')
                        continue;
                /* 
                 * If the file's type is included in the filter,
                 * add it to the list. 
                 */
                if ((F_TYPE(dstat.st_mode) == F_REG)) {
                        struct dlist_node *new;
                        new = calloc(1, sizeof(struct dlist_node));
                        strcpy(new->filename, dirp->d_name);
                        list_add(head, &new->node);
                }
        }
        rewinddir(dir);
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
        struct nav_t nav;
        DIR *dir;

        dir = opendir(path);

        nav_shift(&nav, path); 

        dlist_load(&list, dir, options);

        list_for_each(&list, tmp, node) {
                printf("%s\n", tmp->filename);
        }

        dlist_del(&list);

        nav_revert(&nav);
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

