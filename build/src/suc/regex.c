#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>

#include "suc.h"
#include "meta.h"
#include "parse.h"
#include "../common/io/file.h"
#include "../common/io/dir.h"
#include "../common/io/shell.h"

#include "../common/error.h"
#include "../common/textutils.h"

#define TEST "*.%.*"


#include "regex.h"


/**
 * kleene
 * ``````
 * Tests whether a string conforms to a pattern containing Kleene stars.
 *
 * @pattern: the pattern to be matched
 * @string : the string to be tested
 * Return  : 1 if match, 0 if not match, -1 if bad pattern, -2 if bad string. 
 */
int kleene(const char *pat, const char *str)
{
        top:

        /* Seek and check non-star characters */
        while (*pat != '*') {
                if (*str == *pat) {
                        str++;
                        pat++;
                        if (*pat == '\0') {
                                goto out;
                        }
                } else {
                        goto out;
                }
        }

        /* Move the cursor to the character after
         * the asterisk, the "stop" character where
         * wildcard matching will halt. */
        if (*pat == '*') pat++;

        /* Move the string cursor until it reaches
         * the delimiter, or reaches the NUL. */
        while (*str != *pat) {
                if (*str == '\0') {
                        goto out;
                } else {
                        str++;
                }
        }

        goto top;

        out:
        return (*str == *pat) ? 1 : 0;
}



void globber(struct glob_t *glob, const char *str, int argc, char **argv) 
{
        const char *delim;
        const char *cur;
        const char *arg = NULL;
        int j=0;
        int n=0;
        #if defined(GLOB_DEBUG)
        int start=0;
        int final=0;
        #endif

        cur = argv[n++];

        string_traverse:

        /* Seek up to the first * or % */
        while (*cur != '\0' && *cur != '*' && *cur != '%') cur++;

        /* ------------------------------------------------- KLEENE STAR */
        if (*cur == '*') {
                while (*cur == '*') cur++; // Seek first non-'*'

                delim = cur;

                /* Copy the string up to the delimiter. */
                while (*str != '\0') {
                        glob->big[j++] = *str;

                        if (*str == *delim)
                                break;  // We hit the delimiter.
                        else
                                str++;
                }
        }

        /* ------------------------------------------------- REPLACEMENT */
        if (*cur == '%') {

                cur++;
                delim = cur;

                arg = argv[n++];

                /* Copy argument into glob buffer */
                while (arg && *arg != '\0')
                        glob->big[j++] = *(arg++);

                /* Add the delimiter to the string. */
                glob->big[j++] = *delim;

                /* Fast forward the string to the delimiter. */
                while (*str != '\0') {
                        if (*str == *delim)
                                break;
                        else
                                str++;
                }
        }

        #if defined(GLOB_DEBUG)
        printf("delimit %d: %c\n", i, *delim);
        #endif
        
        #if defined(GLOB_DEBUG)
        final = j;
        printf("globbed %d: %*s\n", i++, start-final, &glob->big[start]);
        start = final;
        #endif

        /* Doesn't conform to the format string. */
        if (*str != *delim)
                goto string_malformed;

        /* Not finished expanding. */
        else if (*str != '\0') {
                str++; // move past delimiter.
                goto string_traverse;
        }

        /* Everything expanded. */
        else
                goto string_terminate;

        string_terminate:
        glob->big[j] = '\0';
        #if defined(GLOB_DEBUG)
        printf("Final: %s\n", glob->big);
        #endif
        return;

        string_malformed:
        #if defined(GLOB_DEBUG)
        printf("String does not match format.\n");
        #endif
        memset(glob->big, '\0', PATHSIZE);
        return;
} 



int op_fmt(void *self, char **filename)
{
        struct op_t *op = (struct op_t *)self;
        struct glob_t glob = {};
        int n=0;
        const char *p;
        static bool init = false;
        static char *arg[20];
        static char buf[PATHSIZE];

        if (!init) {
                slcpy(buf, op->operand, PATHSIZE);

                for (p = strtok(buf, ",");
                     p;
                     p = strtok(NULL, ","))
                {
                        while (*p == ' ') p++;
                        arg[n++] = sldup(p, PATHSIZE);
                }
                init = true;
        }

        globber(&glob, *filename, n, (char **)arg);

        if (glob.big[0] != '\0')
                slcpy(*filename, glob.big, PATHSIZE);

        return 0;
}

        










        

