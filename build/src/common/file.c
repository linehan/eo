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
#include "textutils.h"

/******************************************************************************
 * GENERAL FILE OPERATIONS 
 * 
 * Small functions that safely handle common file operations such as opening
 * and closing a file. They provide a layer of security as well as error
 * reporting.
 ******************************************************************************/

/**
 * sopen -- open a stream pointer to the file identified by 'path' safely
 * @path: path to the desired file 
 * @mode: mode to open file with
 * Returns a pointer to a FILE stream
 */
FILE *sopen(const char *path, const char *mode)
{
        FILE *file;

        if (file = fopen(path, mode), file == NULL)
                bye("Could not open %s", path);

        return file;
}


/**
 * sclose -- close a stream pointer safely
 * @file: pointer to a file stream 
 * Returns nothing
 */
void sclose(FILE *file)
{
        if (fclose(file) == EOF)
                bye("Could not close file");
}

/******************************************************************************
 * FILE PREDICATES 
 * 
 * Queries that can be applied to files. There are three kinds of files that
 * may be queried: open files for which the caller has a file descriptor,
 * open files for which the caller has a stream pointer, and files that may
 * or may not be open, for which the caller has a pathname. 
 ******************************************************************************/
/**
 * exists -- test if a pathname is valid (i.e. the file it names exists)
 * @path: pathname to test
 * Returns true if pathname is valid, else returns false.
 */
bool exists(const char *path)
{
        struct stat buf;
        return ((stat(path, &buf) == -1) && (errno == ENOENT)) ? false : true;
}


/**
 * ftype -- return the type of a file
 * @path: pathname of the file to be typed
 * Returns a type value, one of the macros F_xxx defined above.
 */
int ftype(const char *path)
{
        struct stat statbuf;

        if ((stat(path, &statbuf) == -1))
                bye("ftype: Could not stat file %s", path);

        return F_TYPE(statbuf.st_mode);
}


/**
 * sperm -- format file information as a string, e.g. "drwxr-xr-x"
 * @mode: the file mode value (the st_mode member of a struct stat)
 * Returns a statically-allocated string formatted as seen above.
 *
 * NOTE 
 * I did not name this function. The original version is part of the
 * standard library in Solaris, but although it is referenced in the
 * example program given in man(3) stat, sperm is not included in most 
 * Unixes anymore. The disappointing consequence is that man sperm,
 * rather than satisfying one's curiosity, tends rather to deepen the 
 * ambiguity and doubt which surrounds the whole situation.
 */
const char *sperm(__mode_t mode) 
{
        static char local_buf[16] = {0};
        int i = 0;
        
        /* File type */
        switch (F_TYPE(mode)) {
                case F_REG:   local_buf[i++] = '-'; break;
                case F_DIR:   local_buf[i++] = 'd'; break;
                case F_LINK:  local_buf[i++] = 'l'; break;
                case F_SOCK:  local_buf[i++] = 's'; break;
                case F_PIPE:  local_buf[i++] = 'p'; break;
                case F_CHAR:  local_buf[i++] = 'c'; break;
                case F_BLOCK: local_buf[i++] = 'b'; break;
                default:      local_buf[i++] = '?'; break;
        }

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


/****************************************************************************** 
 * TEXT FILE PARSING
 * 
 * Provide easy facilities for the most common parsing situation in Unix, 
 * in which a text file exists as a list of tuples, each representing an 
 * identifier and a value which is bound to the identifier, with each
 * tuple in the list being separated by a newline character, i.e. on its
 * own line.
 *
 * Such files also commonly contain comments which are not to be parsed,
 * and these are delimited by a comment character.
 *
 ******************************************************************************/
/**
 * get_tokenf -- return a token from the file at 'path'
 * @dest : the destination buffer (token value will be placed here)
 * @token: the token to be scanned for
 * @B    : the breakpoint character (separates tuples)
 * @S    : the separator between identifier and value of the tuple
 * @C    : the comment delimiter character
 * @path : the path of the file to be parsed
 */
void get_tokenf(char *dst, char B, char S, char C, const char *tok, const char *path)
{
        char buffer[LINESIZE];
        char *pruned;
        size_t offset;
        FILE *file;
        
        file = sopen(path, "r");

        while (fgets(buffer, LINESIZE, file)) {
                /* Remove leading and trailing whitespace */
                trimws(buffer);
                /* If line begins with comment character, continue */
                if (buffer[0] == C)
                        continue;
                /* If the token exists in the line */
                if (strstr(buffer, tok)) {
                        /* Calculate offset of the token */
                        offset = strlen(tok) + 1;
                        /* Prune the token text from the return string */ 
                        pruned = &buffer[offset];
                        /* 
                         * If any comment character exists in the line,
                         * replace it with the separator character, so
                         * that the line is effectively truncated.
                         */
                        chrswp(pruned, C, S, strlen(pruned));

                        snprintf(dst, LINESIZE, "%s", pruned);
                        break;
                }
        }
        sclose(file);
}


/**
 * token -- return token at 'path' as a statically allocated string
 * @path : the path of the file to be parsed
 * @token: the token to be scanned for
 */
char *tokenf(char B, char S, char C, const char *tok, const char *path)
{
        static char buffer[LINESIZE];
        get_tokenf(buffer, B, S, C, tok, path);
        return buffer;
}



/****************************************************************************** 
 * DIRECTORY/FILE ITERATION 
 * 
 * Flexible iteration over the files in a directory, ensuring that logic 
 * can receive them in a serial fashion, and thus simplify its design
 * considerations.
 ******************************************************************************/

/**
 * filecount -- count the number of files in a directory
 * @dir   : directory
 * @filter: filters the files included in the total
 * Returns the number of files in 'dir' passing filter. DIR stream is rewound.
 */
int filecount(DIR *dir, int filter)
{
        struct dirent *dirp;
        struct stat dstat;
        int count=0;

        while ((dirp = readdir(dir)) != NULL) {
                /*
                 * If we cannot stat a file, we move on.
                 */
                if (stat(dirp->d_name, &dstat) == -1)
                        continue;
                /* 
                 * If the F_HID filter is not set, and the file
                 * is is hidden, i.e. preceded by a '.', move on.
                 */
                if (!hasvalue(filter, F_HID) && dirp->d_name[0] == '.')
                                continue;
                /* 
                 * If the file's type is included in the filter,
                 * increment the counter. 
                 */
                if ((hasvalue(filter, F_TYPE(dstat.st_mode))))
                                count++;
        }
        rewinddir(dir);
        return count;
}


/**
 * getfile -- like strtok
 */
char *getfile(DIR *dir, int filter)
{
        struct dirent *dirp;
        struct stat dstat;
        static DIR *_dir;

        if (dir != NULL)
                _dir = dir;

        while ((dirp = readdir(_dir)) != NULL) {
                /*
                 * If we cannot stat a file, we move on.
                 */
                if (stat(dirp->d_name, &dstat) == -1)
                        continue;
                /* 
                 * If the F_HID filter is not set, and the file
                 * is is hidden, i.e. preceded by a '.', move on.
                 */
                if (!hasvalue(filter, F_HID) && dirp->d_name[0] == '.')
                                continue;
                /* 
                 * If the file's type is included in the filter,
                 * increment the counter. 
                 */
                if ((hasvalue(filter, F_TYPE(dstat.st_mode))))
                                return dirp->d_name;
        }
        rewinddir(_dir);
        return NULL;
}


/*void list_dir(DIR *dir, int options)*/
/*{*/
        /*struct stat   statbuf;*/
        /*struct dirent *ep;*/
        /*struct passwd *pwd;*/
        /*struct group  *grp;*/
        /*struct tm     *tm;*/
        /*char date[256];*/

        /*if (dir) {*/
                /*while ((ep = readdir(dir)) != NULL) {*/
                        /*if (stat(ep->d_name, &statbuf) == -1)*/
                                /*continue;*/

                        /*[> -- hidden files -- <]*/
                        /*if (!(options & F_HID)) {*/
                                /*if (ep->d_name[0] == '.')*/
                                        /*continue;*/
                        /*}*/

                        /*[> -- permissions -- <]*/
                        /*if (options & LPRM)*/
                                /*printf("%10.10s ", sperm(statbuf.st_mode));*/

                        /*[> -- user -- <]*/
                        /*if (options & LUSR) { */
                                /*if (pwd = getpwuid(statbuf.st_uid), pwd)*/
                                        /*printf("%.8s ", pwd->pw_name);*/
                                /*else*/
                                        /*printf("%d ", statbuf.st_uid);*/
                        /*}*/

                        /*[> -- group -- <] */
                        /*if (options & LGRP) {*/
                                /*if (grp = getgrgid(statbuf.st_gid), grp)*/
                                        /*printf("%-8.8s ", grp->gr_name);*/
                                /*else*/
                                        /*printf("%-8d ", statbuf.st_gid);*/
                        /*}*/

                        /*[> -- filesize -- <]*/
                        /*if (options & LSIZ)*/
                                /*printf("%9jd ", (intmax_t)statbuf.st_size);*/

                        /*[> -- date & time -- <]*/
                        /*if (options & LDAT) {*/
                                /*tm = localtime(&statbuf.st_mtime);*/
                                /*strftime(date, 256, nl_langinfo(D_T_FMT), tm);*/
                                /*printf("%s ", date);*/
                        /*}*/

                        /*[> -- filename -- <]*/
                        /*if (options & LNAM) {*/
                                /*printf("%s ", ep->d_name);*/
                        /*}*/

                        /*printf("\n");*/
                /*}*/
                /*rewinddir(dir);*/
        /*} else {*/
                /*bye("Couldn't open directory");*/
        /*}*/
/*}*/

