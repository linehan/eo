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


/* drwxr-xr-x */
#define PUMP_DIR_MODE ((S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))


/* Pathnames indexed by file tag */
#define PUMP_DIR   ".pump" 
#define PUMP_CONF  ".pump/config"
#define PUMP_LOGIC ".pump/logic"
static const char *PUMP_PATH[]={ PUMP_DIR, PUMP_CONF, PUMP_LOGIC };
static const char *filename[]={ "pump directory", "config", "logic" };


/*void config_token(char *buf, const char *token)*/
/*{*/
        /*char buffer[1024];*/


/**
 * load_cwd -- fill out the current working directory (CWD)
 */
void load_cwd(void)
{
        if (getcwd(CWD, 1024) == NULL)
                bye("Could not stat working directory.");
}


/**
 * make_pump_dir -- create the .pump directory to hold pump metadata
 */
void make_pump(void)
{
        if (mkdir(PUMP_DIR, PUMP_DIR_MODE) == -1)
                bye("Could not create pump directory.");
}


/**
 * pump_open -- get a stream pointer to the file identified by 'tag'
 * @tag : pump file identifier
 * @mode: mode to open file with
 */
FILE *pump_open(enum files tag, const char *mode)
{
        FILE *file;

        if (file = fopen(PUMP_PATH[tag], mode), file == NULL)
                bye("Could not open %s", filename[tag]);

        return file;
}


/**
 * pump_close -- close a stream pointer 
 * @file: pump file stream pointer
 */
void pump_close(FILE *file)
{
        if (fclose(file) == EOF)
                bye("Could not close file");
}


/**
 * is_pump -- test if the current working directory is a pump
 */
bool is_pump(void)
{
        static char pumpdir[1024];
        struct stat buffer;

        sprintf(pumpdir, "%s/%s", CWD, PUMP_PATH[PUMPDIR]);

        if (stat(pumpdir, &buffer) == -1) {
                if (errno == ENOENT)
                        return false;
                else
                        bye("Cannot stat pump directory");
        }
        return true;
}

