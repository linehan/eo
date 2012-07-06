#ifndef _GLOB_H
#define _GLOB_H

struct glob_t {
        int n;
        char big[PATHSIZE];
};


void globber(struct glob_t *glob, const char *globformat, const char *str, const char *rep);

#endif
