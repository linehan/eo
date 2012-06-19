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
#include <locale.h>
#include <langinfo.h>

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


/**
 * sperm -- return file permissions as a string e.g. 'drwxr-xr-x'
 * @mode: the file mode value
 */
const char *sperm(__mode_t mode) 
{
        static char local_buf[16] = {0};
        int i = 0;
        
        /* Directory or regular file */
        local_buf[i] = (S_ISDIR(mode)) ? 'd' : '-'; i++;

        /* User permissions */
        local_buf[i] = ((mode & S_IRUSR)==S_IRUSR) ? 'r' : '-'; i++;
        local_buf[i] = ((mode & S_IWUSR)==S_IWUSR) ? 'w' : '-'; i++;
        local_buf[i] = ((mode & S_IXUSR)==S_IXUSR) ? 'x' : '-'; i++;

        /* Group permissions */
        local_buf[i] = ((mode & S_IRGRP)==S_IRGRP) ? 'r' : '-'; i++;
        local_buf[i] = ((mode & S_IWGRP)==S_IWGRP) ? 'w' : '-'; i++;
        local_buf[i] = ((mode & S_IXGRP)==S_IXGRP) ? 'x' : '-'; i++;

        /* Other permissions */
        local_buf[i] = ((mode & S_IROTH)==S_IROTH) ? 'r' : '-'; i++;
        local_buf[i] = ((mode & S_IWOTH)==S_IWOTH) ? 'w' : '-'; i++;
        local_buf[i] = ((mode & S_IXOTH)==S_IXOTH) ? 'x' : '-';

        return local_buf;
}


int filecount(DIR *dir, int options)
{
        struct dirent *dirp;
        struct stat dstat;
        int n=0;

        while ((dirp = readdir(dir)) != NULL) {
                if (stat(dirp->d_name, &dstat) == -1)
                        continue;

                if (!(options & LHID)) {
                        if (dirp->d_name[0] == '.')
                                continue;
                }
                if ((options & LDIR)) {
                        if (S_ISDIR(dstat.st_mode))
                                n++;
                }
                if ((options & LREG)) {
                        if (S_ISREG(dstat.st_mode))
                                n++;
                }
        }

        rewinddir(dir);
        return n;
}

/**
 * getfile -- like strtok
 */
char *getfile(DIR *dir, int options)
{
        struct dirent *dirp;
        struct stat dstat;
        static DIR *_dir;
        int n=0;

        if (dir != NULL)
                _dir = dir;

        while ((dirp = readdir(_dir)) != NULL) {
                if (stat(dirp->d_name, &dstat) == -1)
                        continue;

                if (!(options & LHID)) {
                        if (dirp->d_name[0] == '.')
                                continue;
                }
                if ((options & LDIR)) {
                        if (S_ISDIR(dstat.st_mode))
                                return dirp->d_name;
                }
                if ((options & LREG)) {
                        if (S_ISREG(dstat.st_mode))
                                return dirp->d_name;
                }
        }
        rewinddir(_dir);
        return NULL;
}


void list_dir(DIR *dir, int options)
{
        struct stat   statbuf;
        struct dirent *ep;
        struct passwd *pwd;
        struct group  *grp;
        struct tm     *tm;
        char date[256];

        if (dir) {
                while ((ep = readdir(dir)) != NULL) {
                        if (stat(ep->d_name, &statbuf) == -1)
                                continue;

                        /* -- hidden files -- */
                        if (!(options & LHID)) {
                                if (ep->d_name[0] == '.')
                                        continue;
                        }

                        /* -- permissions -- */
                        if (options & LPRM)
                                printf("%10.10s ", sperm(statbuf.st_mode));

                        /* -- user -- */
                        if (options & LUSR) { 
                                if (pwd = getpwuid(statbuf.st_uid), pwd)
                                        printf("%.8s ", pwd->pw_name);
                                else
                                        printf("%d ", statbuf.st_uid);
                        }

                        /* -- group -- */ 
                        if (options & LGRP) {
                                if (grp = getgrgid(statbuf.st_gid), grp)
                                        printf("%-8.8s ", grp->gr_name);
                                else
                                        printf("%-8d ", statbuf.st_gid);
                        }

                        /* -- filesize -- */
                        if (options & LSIZ)
                                printf("%9jd ", (intmax_t)statbuf.st_size);

                        /* -- date & time -- */
                        if (options & LDAT) {
                                tm = localtime(&statbuf.st_mtime);
                                strftime(date, 256, nl_langinfo(D_T_FMT), tm);
                                printf("%s ", date);
                        }

                        /* -- filename -- */
                        if (options & LNAM) {
                                printf("%s ", ep->d_name);
                        }

                        printf("\n");
                }
                rewinddir(dir);
        } else {
                bye("Couldn't open directory");
        }
}



