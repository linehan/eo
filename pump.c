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

#define USE_ERRNO_H
#include "util.h"


/* EXCEPTION HANDLING
``````````````````````````````````````````````````````````````````````````````*/
#define bye(...) { if (VA_NUM_ARGS(__VA_ARGS__) == 1)  \
                        abort_report(__VA_ARGS__, ""); \
                   else                                \
                        abort_report(__VA_ARGS__);  }  \
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



void print_usage(void)
{
        printf("pump usage statement\n");
        exit(0);
}

char cwd[1024];

#define STRCMP(a,b) (strcmp((a),(b)) == 0) ? true : false

/* 
 * File modes
 *      User
 *      ````    S_IRUSR
 *              S_IWUSR
 *              S_IXUSR
 *              S_IRWXU (Equivalent to '(S_IRUSR | S_IWUSR | S_IXUSR)').
 *      Group
 *      `````   S_IRGRP
 *              S_IWGRP
 *              S_IXGRP
 *              S_IRWXG (Equivalent to '(S_IRGRP | S_IWGRP | S_IXGRP)').
 *      Other
 *      `````   S_IROTH
 *              S_IWOTH
 *              S_IXOTH
 *              S_IRWXO (Equivalent to '(S_IROTH | S_IWOTH | S_IXOTH)').
 */
// pump_dir permissions: drwxr-xr-x
#define PUMP_DIR "./.pump"
#define PUMP_CONF "./.pump/pump.conf"
#define PUMP_LOGIC "./.pump/pump.logic"
#define PUMP_DIR_MODE ((S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))


void pump_init(void)
{
        FILE *conf;

        if (getcwd(cwd, 1024) == NULL)
                bye("Could not stat working directory.");
        if (mkdir(PUMP_DIR, PUMP_DIR_MODE) == -1)
                bye("Could not create pump directory.");
        if (conf = fopen(PUMP_CONF, "w+"), conf == NULL)
                bye("Could not create pump.conf");

        fprintf(conf, "#pump configuration\n basedir %s\n", cwd);

        if (fclose(conf) == EOF)
                bye("Could not close pump.conf");
}


int main(int argc, char *argv[]) 
{
        if (!argv[1])
                print_usage();
        if (STRCMP(argv[1], "init"))
                pump_init();

        return 0;
}
