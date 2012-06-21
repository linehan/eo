#ifndef _MY_FILE_LIB_H
#define _MY_FILE_LIB_H

#include "../common/util.h"
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>

#define PATHSIZE 255
#define LINESIZE 1024

bool exists(const char *path);

FILE *sopen(const char *path, const char *mode);
void sclose(FILE *file);
const char *homedir(uid_t uid);

void list_dir(DIR *dir, int options);

int filecount(DIR *dir, int options);
char *getfile(DIR *dir, int options);

void get_tokenf(char *dst, char B, char S, char C, const char *tok, const char *path);
char *tokenf(char B, char S, char C, const char *tok, const char *path);

/* 
 * Macros for most common scenario
 */
#define get_token(dst, tok, path) get_tokenf(dst, '\n', ' ', '#', tok, path)
#define token(tok, path)          tokenf('\n', ' ', '#', tok, path)


/******************************************************************************
 * FILETYPE EXTENSIONS
 *
 * The following macros should be defined in <sys/stat.h>:
 *
 *                 (I think this is a typo in the kernel source)
 *                  /
 * #define S_IFMT  00170000   Mask the mode bytes describing file type
 * #define S_IFSOCK 0140000   Socket
 * #define S_IFLNK  0120000   Symlink
 * #define S_IFREG  0100000   Regular file
 * #define S_IFBLK  0060000   Block device
 * #define S_IFDIR  0040000   Directory
 * #define S_IFCHR  0020000   Character device
 * #define S_IFIFO  0010000   FIFO (named pipe)
 *
 * #define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
 * #define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
 * #define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
 * #define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
 * #define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
 * #define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
 * #define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
 *
 * Another macro seems to suggest itself here, although it's
 * not actually included. This library provides it because it
 * makes code easier to read:
 */
#define F_TYPE(mode) ((mode) & S_IFMT)
/*
 * Which expands to the type value used in the mode quantity 'm'.
 * Note that the value from this macro is NOT compatible as an
 * argument to the S_ISxxx macros defined in <sys/stat.h>.
 * Those macros perform the equivalent of S_TYPE(m) internally.
 *
 * I also find the default macros difficult to read and comprehend
 * at a glance. There is too much unnecessary baggage being supplied
 * before you get to the useful information. The following mapping is
 * used in this source file:
 */
#define F_PIPE  S_IFIFO
#define F_SOCK  S_IFSOCK
#define F_LINK  S_IFLNK
#define F_REG   S_IFREG
#define F_BLOCK S_IFBLK
#define F_CHAR  S_IFCHR
#define F_DIR   S_IFDIR
/* 
 * Sometimes we want to signal that we wish to test for a
 * hidden file, whatever the implementation may define that
 * as. So as not to step on the toes of the other filetype
 * macros, we define another macro
 */
#define F_HID (0160000)   // Filter for hidden files
/* 
 * That signals the intention to test for hidden files, which
 * again must be implementation-defined. This is just to
 * allow the option bit to be OR'ed with any of the other type
 * options, without loss of any information.
 *
 * These are used to operate on the st_mode field of the stat structure,
 * also defined in <sys/stat.h>
 *
 * struct stat {
 *      unsigned long   st_dev;         // Device.
 *      unsigned long	st_ino;		// File serial number.
 *      unsigned int	st_mode;	// File mode.
 *      unsigned int	st_nlink;	// Link count.
 *      unsigned int	st_uid;		// User ID of the file's owner.
 *      unsigned int	st_gid;		// Group ID of the file's group.
 *      unsigned long	st_rdev;	// Device number, if device.
 *      unsigned long	__pad1;
 *      long		st_size;	// Size of file, in bytes.
 *      int		st_blksize;	// Optimal block size for I/O.
 *      int		__pad2;
 *      long		st_blocks;	// Number 512-byte blocks allocated.
 *      long		st_atime;	// Time of last access.
 *      unsigned long	st_atime_nsec;
 *      long		st_mtime;	// Time of last modification.
 *      unsigned long	st_mtime_nsec;
 *      long		st_ctime;	// Time of last status change.
 *      unsigned long	st_ctime_nsec;
 *      unsigned int	__unused4;
 *      unsigned int	__unused5;
 * };
 *
 ******************************************************************************/

#endif
