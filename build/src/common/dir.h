#ifndef _DIR_LISTING_H
#define _DIR_LISTING_H

#include "list/list.h"
#include <stdbool.h>

void dlist_print(const char *path, int options);
void dlist_load(struct list_head *head, DIR *dir, int filter);
void dlist_del(struct list_head *head);
void dlist_diff(struct list_head *diff, struct list_head *old, struct list_head *new);

int filecount(DIR *dir, int options);
const char *getfile(DIR *dir, int options);
const char *getdiff(DIR *dir, int filter);


struct breadcrumb_t {
        char home[PATHSIZE];
        bool away;
};

void cwd_mark  (struct breadcrumb_t *dirnav);
void cwd_shift (struct breadcrumb_t *dirnav, const char *path);
void cwd_revert(struct breadcrumb_t *dirnav);

#endif
