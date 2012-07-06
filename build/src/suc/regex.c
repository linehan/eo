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

#define GLOB_DEBUG

void globber(struct glob_t *glob, const char *globformat, const char *str, const char *rep)
{
        const char *delim;
        const char *cur;
        size_t i=0;
        size_t j=0;
        #if defined(GLOB_DEBUG)
        size_t start = 0;
        size_t final = 0;
        #endif

        cur = globformat;

        string_traverse:

        while (*cur != '\0' && *cur != '*') cur++; // Seek up to first '*'
        while (*cur == '*')                 cur++; // Seek first non-'*'

        delim = cur; // set the delimiter.
        
        #if defined(GLOB_DEBUG)
        printf("delimit %d: %c\n", (int)i, *delim);
        #endif
        
        /* Copy the string up to the delimiter. */
        while (*str != '\0') {

                glob->big[j++] = *str;

                if (*str == *delim)
                        break;  // We hit the delimiter.
                else
                        str++;
        }

        #if defined(GLOB_DEBUG)
        final = j;
        printf("globbed %d: %*s\n", i++, (int)start-final, &glob->big[start]);
        start = final;
        #endif

        /* String doesn't conform to structure of the format string. */
        if (*str != *delim)
                goto string_malformed;

        /* String is not finished expanding. */
        else if (*str != '\0') {
                str++; // move past delimiter.
                goto string_traverse;
        }

        /* Entire string has been expanded. */
        else
                goto string_terminate;

        string_terminate:
        glob->big[j] = '\0';
        #if defined(GLOB_DEBUG)
        printf("FINAL: %s\n", glob->big);
        #endif
        return;

        string_malformed:
        #if defined(GLOB_DEBUG)
        printf("String does not match format.\n");
        #endif
        memset(glob->big, '\0', PATHSIZE);
        return;
} 
        

