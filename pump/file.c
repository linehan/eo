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
#include "../common/error.h"
#include "../common/util.h"
#include "../common/textutils.h"


/* drwxr-xr-x */
#define PUMP_DIR_MODE ((S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))

/* Pathnames indexed by file tag */
#define pump_stem   "/.pump" 
#define config_stem "/.pump/config"
#define logic_stem  "/.pump/logic"


/******************************************************************************
 * GENERAL FILE OPERATIONS 
 * 
 * Small functions that safely handle common file operations performed
 * by pump and pumpd. They provide a layer of security as well as error
 * reporting.
 ******************************************************************************/

/**
 * load_env -- fill out the current working directory struture 
 * @environ: will contain the current environment 
 */
void load_env(struct env_t *environ)
{
        if (getcwd(environ->cwd, PATHSIZE) == NULL)
                bye("Could not stat working directory.");

        snprintf(environ->pump,   PATHSIZE, "%s%s", environ->cwd, pump_stem);
        snprintf(environ->config, PATHSIZE, "%s%s", environ->cwd, config_stem);
        snprintf(environ->logic,  PATHSIZE, "%s%s", environ->cwd, logic_stem);
}


/**
 * pumpfile_open -- get a stream pointer to the file identified by 'path' 
 * @path: path to the desired file 
 * @mode: mode to open file with
 */
FILE *pumpfile_open(const char *path, const char *mode)
{
        FILE *file;

        if (file = fopen(path, mode), file == NULL)
                bye("Could not open %s", path);

        return file;
}


/**
 * pumpfile_close -- close a stream pointer 
 * @file: file stream pointer
 */
void pumpfile_close(FILE *file)
{
        if (fclose(file) == EOF)
                bye("Could not close file");
}


/****************************************************************************** 
 * PUMP METADATA FILES 
 * 
 * The path argument accepted by all of these refers to a working
 * directory which is the root of the directory that contains the
 * .pump subfolder, NOT the actual path of a particular file.
 ******************************************************************************/

/**
 * make_pump -- create a ./.pump directory to hold pump metadata
 * @path: the root directory in which to create .pump
 * 
 * It should be noted that this function sets the diagnostic condition 
 * of the is_pump() predicate. Whether a given directory is or is not 
 * a pump is judged entirely on the existence of a .pump subdirectory.
 */
void make_pump(const char *path)
{
        if (mkdir(CONCAT(path, pump_stem), PUMP_DIR_MODE) == -1)
                bye("Could not create pump directory.");
}


/**
 * is_pump -- test if 'path' contains a .pump subdirectory (is a pump)
 * @path: the directory in which to look for a .pump subdirectory
 */
bool is_pump(const char *path)
{
        struct stat buffer;

        if (stat(CONCAT(path, pump_stem), &buffer) == -1) {
                if (errno == ENOENT)
                        return false;
                else
                        bye("Cannot stat pump directory");
        }
        return true;
}


/**
 * exists -- test if a file exists 
 * @path: the root directory of the pump 
 */
bool exists(const char *path)
{
        struct stat buf;
        return ((stat(path, &buf) == -1) && (errno == ENOENT)) ? false : true;
}


/****************************************************************************** 
 * READING THE PUMP CONFIG FILE 
 * 
 * These functions provide easy access to the configuration
 * file and other metadata.
 ******************************************************************************/
/**
 * get_token -- return a token from the file at 'path'
 * @dest : the destination buffer (token value will be placed here)
 * @path : the path of the file to be parsed
 * @token: the token to be scanned for
 */
void get_token(char *dest, const char *token, const char *path)
{
        #define COMMENT '#'
        char buffer[LINESIZE];
        char *pruned;
        size_t offset;
        FILE *file;
        
        file = pumpfile_open(path, "r");

        while (fgets(buffer, LINESIZE, file)) {
                if (buffer[0] == COMMENT)
                        continue;
                if (strstr(buffer, token)) {
                        offset = strlen(token) + 1;
                        pruned = &buffer[offset];
                        terminate(pruned, COMMENT, strlen(pruned));
                        trimws(pruned);
                        snprintf(dest, LINESIZE, "%s", pruned);
                        break;
                }
        }
        pumpfile_close(file);
}


/**
 * token -- return token at 'path' as a statically allocated string
 * @path : the path of the file to be parsed
 * @token: the token to be scanned for
 */
char *token(const char *token, const char *path)
{
        static char buffer[LINESIZE];
        get_token(buffer, token, path);
        return buffer;
}


/**
 * readconfig -- fill a pumpconfig structure with the config file at 'path'
 * @config: pumpconfig struct
 * @path  : path to the config file to be parsed
 */
void readconfig(struct pumpconfig_t *config, const char *path)
{
        get_token(config->name, "name", path);
        get_token(config->desc, "desc", path);
        get_token(config->base, "base", path);
        get_token(config->sha2, "sha2", path);
        get_token(config->link, "link", path);
        get_token(config->wait, "wait", path);
}


/**
 * writeconfig -- write a pumpconfig structure to a config file at 'path'
 * @config: pumpconfig struct
 * @path  : path to the config file to be written
 */
void writeconfig(struct pumpconfig_t *config, const char *path)
{
        FILE *configfile;

        configfile = pumpfile_open(path, "w+");
        fprintf(configfile, 
                        "# --------------------------\n"
                        "# PUMP CONFIGURATION\n"
                        "# --------------------------\n\n"
                        "# Pump name\n"
                        "name \"%s\"\n\n"
                        "# Pump description\n"
                        "desc \"%s\"\n\n"
                        "# Directory to be pumped\n"
                        "base %s\n\n"
                        "# Identifies pump to the pump daemon\n"
                        "sha2 %s\n\n"
                        "# Link to external logic\n"
                        "link \"%s\"\n\n"
                        "# Delay between pumps (in seconds)\n"
                        "wait %s\n\n",
                config->name, 
                config->desc, 
                config->base, 
                config->sha2,
                config->link,
                config->wait);

        pumpfile_close(configfile);
}


void pumpconfig(struct pumpconfig_t *config, 
                char *n, char *d, char *b, char *s, char *l, char *w)
{
        char *name, *desc, *base, *sha2, *link, *wait;

        name = n ? n : "";
        desc = d ? d : "";
        base = b ? b : "";
        sha2 = s ? s : "";
        link = l ? l : "";
        wait = w ? w : "";

        snprintf(config->name, LINESIZE, "%s", name);
        snprintf(config->desc, LINESIZE, "%s", desc);
        snprintf(config->base, LINESIZE, "%s", base);
        snprintf(config->sha2, LINESIZE, "%s", sha2);
        snprintf(config->link, LINESIZE, "%s", link);
        snprintf(config->wait, LINESIZE, "%s", wait);
}


void setconfig(const char *path, const char *token, const char *value)
{
        struct pumpconfig_t config;

        readconfig(&config, path);

        if (STRCMP(token, "name"))
                snprintf(config.name, LINESIZE, "%s", value);
        else if (STRCMP(token, "desc"))
                snprintf(config.desc, LINESIZE, "%s", value);
        else if (STRCMP(token, "base"))
                snprintf(config.base, LINESIZE, "%s", value);
        else if (STRCMP(token, "sha2"))
                snprintf(config.sha2, LINESIZE, "%s", value);
        else if (STRCMP(token, "link"))
                snprintf(config.link, LINESIZE, "%s", value);
        else if (STRCMP(token, "wait"))
                snprintf(config.wait, LINESIZE, "%s", value);
        else
                bye("Invalid token passed to setconfig()");

        writeconfig(&config, path);
}


/**
 * sperm -- return file permissions as a string e.g. 'drwxr-xr-x'
 * @mode: the file mode value
 */
const char *sperm(__mode_t mode) 
{
        static char local_buf[16] = {0};
        int i = 0;
        
        /* File type */
        if (S_ISREG(mode))
                local_buf[i] = '-';
        else if (S_ISDIR(mode))
                local_buf[i] = 'd';
        else if (S_ISCHR(mode))
                local_buf[i] = 'c';
        else if (S_ISBLK(mode))
                local_buf[i] = 'b';
        else if (S_ISFIFO(mode))
                local_buf[i] = 'p';
        else if (S_ISLNK(mode))
                local_buf[i] = 'l';
        else if (S_ISSOCK(mode))
                local_buf[i] = 's';
        else
                local_buf[i] = '?';

        i++;

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

                if (!(options & F_HID)) {
                        if (dirp->d_name[0] == '.')
                                continue;
                }
                if ((options & F_DIR)) {
                        if (S_ISDIR(dstat.st_mode))
                                n++;
                }
                if ((options & F_REG)) {
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

        if (dir != NULL)
                _dir = dir;

        while ((dirp = readdir(_dir)) != NULL) {
                if (stat(dirp->d_name, &dstat) == -1)
                        continue;

                if (!(options & F_HID)) {
                        if (dirp->d_name[0] == '.')
                                continue;
                }
                if ((options & F_DIR)) {
                        if (S_ISDIR(dstat.st_mode))
                                return dirp->d_name;
                }
                if ((options & F_REG)) {
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
                        if (!(options & F_HID)) {
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



#define SH_GREEN    "\033[01;32m"
#define SH_CYAN     "\033[00;36m"
#define SH_CYANBOLD "\033[01;36m"
#define SH_REDBOLD  "\033[01;31m"
#define SH_MAGBOLD  "\033[01;35m"
#define SH_DEFBOLD  "\033[00;1m"
#define SH_DEFAULT  "\033[00;m"


void pump_info(const char *path)
{
        if (!is_pump(path))
                return;
        printf("%s[%s@%s]%s-pump", SH_MAGBOLD, SH_REDBOLD, SH_MAGBOLD, SH_DEFAULT);
}

