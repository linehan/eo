#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include "error.h"


/**
 * abort_report -- clean up and exit, printing a brief diagnostic report
 * @fmt: a printf-style format string
 * @...: the variable argument list to the format string
 */
void abort_report(const char *fmt, ...)
{
        char buf[1000];
        va_list args;

        /* Write formatted output to stream */
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);

        #ifdef USE_ERRNO_H
        if (errno)
                printf("%s (%d): %s\n", etag[errno], errno, emsg[errno]);
        else
                printf("An error has occured. ");
        #endif

        printf("The handler reported: \"%s\"\n", buf);

        exit(1);
}

