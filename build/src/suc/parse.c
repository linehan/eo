#define __MERSENNE__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

#include "suc.h"
#include "meta.h"
#include "parse.h"
#include "../common/io/file.h"
#include "../common/ipc/daemon.h"
#include "../common/ipc/channel.h"

#include "../common/error.h"
#include "../common/util.h"
#include "../common/configfiles.h"
#include "../common/textutils.h"
#include "../common/lib/bloom/bloom.h"


static const char *SYMBOL[]={":", "{}", "@"};


int ntok(const char *str, const char *tok)
{
        size_t toklen;
        char *sub;

        int count=0;

        toklen = strlen(tok);

        for (sub  = (char *)memem(str, tok);
             sub != NULL;
             sub  = (char *)memem((sub+toklen), tok))
        {
                count++;
        }

        return count;
}






struct parse_t *parse(const char *input)
{
        struct parse_t *new;
        int seg;
        int op;
        char buffer[4096];
        char *segment;
        int n;

        slcpy(buffer, input, 4096);

        new = calloc(1, sizeof(struct parse_t));

        n = ntok(input, ":");
        /* For n tokens, there are n+1 segments (fencepost) */
        new->node = malloc(n+1 * sizeof(char *));

        int i = 0;
        for (segment = strtok(buffer, ":");
             segment != NULL;
             segment = strtok(NULL, ":"))
        {
                new->node[i++] = sdup(segment);
        }
        new->n = i;
                
        return new;
}


         
