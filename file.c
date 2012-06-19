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

#include "file.h"
#include "error.h"
#include "util.h"


#define PUMP_DIR_MODE ((S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
// permissions: drwxr-xr-x
#define PUMP_DIR "./.pump" 
#define PUMP_CONF "./.pump/config"
#define PUMP_LOGIC "./.pump/logic"

static const char *PUMP_PATH[]={
        PUMP_DIR,
        PUMP_CONF,
        PUMP_LOGIC 
};



/*void config_token(char *buf, const char *token)*/
/*{*/
        /*char buffer[1024];*/


void pump_load(void)
{
        /* Get current working directory */
        if (getcwd(CWD, 1024) == NULL)
                bye("Could not stat working directory.");
}

void make_pump(void)
{
        /* Create the .pump directory */
        if (mkdir(PUMP_DIR, PUMP_DIR_MODE) == -1)
                bye("Could not create pump directory.");
}

FILE *pump_open(enum files tag, const char *mode)
{
        FILE *file;

        if (file = fopen(PUMP_PATH[tag], mode), file == NULL)
                bye("Could not open %s", filename[tag]);

        return file;
}


void pump_close(FILE *file)
{
        if (fclose(file) == EOF)
                bye("Could not close file");
}


bool is_pump_directory(void)
{
        static char pumpdir[1024];
        struct stat buffer;

        if (CWD[0] == '\0')
                pump_load();

        sprintf(pumpdir, "%s/.pump", CWD);

        if (stat(pumpdir, &buffer) == -1) {
                if (errno == ENOENT)
                        return false;
                else
                        bye("Cannot stat pump directory");
        }
        return true;
}

