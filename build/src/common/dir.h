#ifndef _DIR_LISTING_H
#define _DIR_LISTING_H

#include "list/list.h"
#include <stdbool.h>

int filecount(DIR *dir, int options);
const char *getfile(DIR *dir, int options);
const char *getdiff(DIR *dir, int filter);

#endif
