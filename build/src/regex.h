#ifndef _GLOB_H
#define _GLOB_H

#include <stdarg.h>

struct glob_t {
        int n;
        char big[PATHSIZE];
};

void globber(struct glob_t *glob, const char *str, int n, char *argv[]);
int op_fmt(void *self, char **filename);
int kleene(const char *pat, const char *str);

#endif
