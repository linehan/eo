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
#include "list.h"


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


/**
 * sunlink -- unlink safely 
 * @path: path of file to be removed
 */
void sunlink(const char *path)
{
        if ((unlink(path)) < 0)
                bye("Could not unlink %s", path);
}


/**
 * srmdir -- remove a directory safely
 * @path: path of the directory to be removed
 */
void srmdir(const char *path)
{
        if ((rmdir(path)) < 0)
                bye("Could not remove directory %s", path);
}


/**
 * smkdir -- create a new directory safely
 * @path: path of the directory to be created
 */
void smkdir(const char *path, int perms)
{
        if ((mkdir(path, perms)) < 0)
                bye("Could not create directory %s", path);
}


/**
 * gethome_uid -- return the home directory of user with 'uid'
 * @uid: uid of the user whose home directory you want
 */
char *gethome_uid(uid_t uid)
{
        static char *path;
        struct passwd *pw;
       
        pw   = getpwuid(uid);
        path = pw->pw_dir;

        return path;
}


/**
 * gethome -- return the home directory set in the environment variable $HOME
 */
char *gethome(void)
{
        static char *path;
        path = getenv("HOME");
        return path;
}


/**
 * rel2abs -- resolve a relative path to an absolute path
 */
void rel2abs(char *rel, char *abs)
{
        if ((getcwd(abs, PATHSIZE)) == NULL)
                bye("Could not stat working directory.");

        strlcat(abs, CONCAT("/", rel), 255);
}


/**
 * is_relpath -- check if path is relative
 */
bool is_relpath(char *path)
{
        return (path[0] == '/') ? false : true;
}





/******************************************************************************
 * FILE CREATION 
 ******************************************************************************/

/**
 * tmpname -- return a temporary name according to a template "test.XXXXXX"
 */
int tempname(char *template)
{
        pid_t val;
        int start;

        val   = getpid();
        start = strlen(template) - 1;

        while (template[start] == 'X') {
                template[start] = '0' + val % 10;
                val /= 10;
                start--;
        }
        return start;
}


int tempdir(char *template)
{
        char name[255];
        int  start;

        strcpy(name, template);

        start = tempname(name);
        if ((mkdir(name, 0700)), errno != EEXIST)
                return 0;
        else
                return -1;
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
const char *getfile(DIR *dir, int filter)
{
        struct dirent *dirp;
        struct stat dstat;
        static DIR *_dir;

        if (dir != NULL)
                _dir = dir;

        while ((dirp = readdir(_dir)), dirp != NULL) {
                /*
                 * If we cannot stat a file, we move on.
                 */
                if (stat(dirp->d_name, &dstat) == -1)
                        continue;
                /* 
                 * If the F_HID filter is not set, and the file
                 * is is hidden, i.e. preceded by a '.', move on.
                 */
                if (dirp->d_name[0] == '.' && !hasvalue(filter, F_HID))
                        continue;
                /* 
                 * If the file's type is included in the filter,
                 * return that filename. 
                 */
                if (hasvalue(filter, F_TYPE(dstat.st_mode)))
                        return dirp->d_name;
        }
        rewinddir(_dir);
        return NULL;
}


struct dirlist_t {
        char filename[PATHSIZE];
        struct list_node node;
};


void list_dir(struct list_head *head, DIR *dir, int filter)
{
        struct dirent *dirp;
        struct stat dstat;

        while ((dirp = readdir(dir)) != NULL) {
                /*
                 * If we cannot stat some file, we move on.
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
                 * add it to the list. 
                 */
                if ((hasvalue(filter, F_TYPE(dstat.st_mode)))) {
                        struct dirlist_t *new;
                        new = calloc(1, sizeof(struct dirlist_t));
                        strlcat(new->filename, dirp->d_name, PATHSIZE);
                        list_add(head, &new->node);
                }
        }
}


void cmp_dir(const char *path, int options)
{
        LIST_HEAD(old);
        LIST_HEAD(new);
        struct dirlist_t *tmp;
        DIR *dir;

        dir = opendir(path);

        list_dir(&old, dir, options);

        list_for_each(&old, tmp, node) {
                printf("%s\n", tmp->filename);
        }
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



/*#define BUFF_SIZE	2048*/


/*int filecmp(char *scr,char *copy);*/
/*int dircmp(char *src, char *copy);*/
/*void *sbuff;*/
/*void *cbuff;*/
/*int filecnt = 0;*/
/*int dircnt = 0;*/


/*int dircmp(char *src, char *copy)*/
/*{*/
	/*struct _finddata_t c_file;*/
	/*long hFile;*/
	/*char srcwc[_MAX_FNAME];*/
	/*int ret = 0;*/
	/*puts(src);*/
	/*strcat(strcpy(srcwc,src),"*.*");*/
	/*hFile = _findfirst(srcwc, &c_file);*/
	/*if(hFile == -1L){*/
		/*printf("A file doesn't exist in this directory.\n");*/
		/*ret = -1;*/
	/*}*/
	/*else{*/
		/*do{*/
			/*char sname[_MAX_FNAME];*/
			/*char cname[_MAX_FNAME];*/
			/*if(strcmp(c_file.name,".")!=0 && strcmp(c_file.name,"..")!=0){*/
				/*strcpy(sname,src);*/
				/*strcat(sname,c_file.name);*/
				/*strcpy(cname,copy);*/
				/*strcat(cname,c_file.name);*/
				/*if(c_file.attrib & _A_SUBDIR){*/
					/*strcat(sname,"\\");*/
					/*strcat(cname,"\\");*/
					/*dircnt++;*/
					/*if(ret = dircmp(sname,cname)) break;*/
				/*}*/
				/*else{*/
					/*filecnt++;*/
					/*if(ret = filecmp(sname,cname)) break;*/
				/*}*/
			/*}*/
		/*}while(_findnext( hFile, &c_file ) == 0);*/
		/*_findclose(hFile);*/
	/*}*/
	/*return ret;*/
/*}*/

/*int filecmp(char *src,char *copy){*/
	/*int filesizeS, filesizeC;*/
	/*int sh,ch;*/
	/*int Sreadsize, Creadsize;*/
	/*int ret = 0;*/

	/*sh = _sopen(src, _O_RDONLY, _SH_DENYWR);*/
	/*if(sh == -1){*/
		/*printf("\"%s\" doesn't open it.\n", src);*/
		/*return -1;*/
	/*}*/
	/*ch = _sopen(copy, _O_RDONLY, _SH_DENYWR);*/
	/*if(ch == -1){*/
		/*printf("\"%s\" doesn't open it.\n", copy);*/
		/*_close(sh);*/
		/*return -1;*/
	/*}*/
	/*_setmode(sh, _O_BINARY);*/
	/*_setmode(ch, _O_BINARY);*/
	/*filesizeS = _filelength(sh);*/
	/*filesizeC = _filelength(ch);*/
	/*if(filesizeS != filesizeC){*/
		/*printf("The size of the file doesn't correspond.\n");*/
		/*printf("\"%s\" : %d byte\n", src, filesizeS);*/
		/*printf("\"%s\" : %d byte\n", copy, filesizeC);*/
		/*ret = -1;*/
	/*}*/
	/*else{*/
		/*for(;;){*/
			/*Sreadsize = _read(sh, sbuff, BUFF_SIZE);*/
			/*if(Sreadsize == -1){*/
				/*printf("\"%s\" read error",src);*/
				/*ret = -1;*/
				/*break;*/
			/*}*/
			/*Creadsize = _read(ch, cbuff, BUFF_SIZE);*/
			/*if(Creadsize == -1){*/
				/*printf("\"%s\" read error (;_;)",copy);*/
				/*ret = -1;*/
				/*break;*/
			/*}*/
			/*if(memcmp(sbuff, cbuff, Sreadsize)){*/
				/*printf("Difference was detected. (T_T) \"%s\" \n", copy);*/
				/*ret = -1;*/
				/*break;*/
			/*}*/
			/*if(Sreadsize != BUFF_SIZE) break;*/
		/*}*/
	/*}*/
	/*_close(sh);*/
	/*_close(ch);*/
	/*return ret;*/
/*}*/

