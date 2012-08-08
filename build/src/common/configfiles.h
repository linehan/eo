#ifndef _PUMP_CONFIG_FILES_H
#define _PUMP_CONFIG_FILES_H

#include "textutils.h"

/******************************************************************************
 * FILES AND DIRECTORIES 
 ******************************************************************************/
/*
 * File permissions
 *
 * When a daemon process is forked, its process group is dissociated
 * from process group of its parent. It is important that if files 
 * are created by a parent process, they are given permissions that
 * will allow the daemon to access them as well, since from the 
 * kernel's perspective, the daemon's process bears no relation to the 
 * owner/creator of the files.
 *
 * Specifically, we want to ensure that processes in the "others"
 * triplet will have read and write access. For directories, this
 * means having execute access, because "entering" a directory is
 * equivalent to executing a file of type "directory". 
 *
 *      (u)ser   process which created file
 *      (g)roup  processes sharing the owner's group
 *      (o)ther  any other process
 *
 *        u   g   o 
 *      ======================================================
 *      -rwx rwx rwx      full permissions
 *      -rw- r-- r--      default permissions for files
 *      -rw- rw- rw-      desired permissions for files
 *      drwx r-- ---      default permissions for directories
 *      drwx r-- r-x      desired permissions for directories
 *      =======================================================
 */
#define CFG_PERMS ((S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
#define DIR_PERMS ((S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH))
#define PERMS (0666)


/*
 * Home directory
 * ``````````````
 * Certain utility files will be placed in a folder in the home directory
 * of the user associated with the process.
 */
static char *HOME = NULL;


#define CFG_NAME (".pipeutils")
#define CFG_PATH (__cfg_path())
/**
 * __cfg_path
 * ``````````
 * Returns the path of the pipeutils configuration directory.
 *
 * NOTES
 * The pump daemon maintains a number of files, all of which are
 * stored in a hidden directory, CFG_STEM, which resides in the
 * directory identified by CFG_PATH. 
 *
 * This is an implementation function that is called through the 
 * CFG_PATH macro defined above.
 */
static inline const char *__cfg_path(void)
{
        static char buf[PATHSIZE];
        if (!HOME) HOME = sdup(gethome());
        snprintf(buf, PATHSIZE, "%s/%s", HOME, CFG_NAME);
        return buf;
}


#define PID_NAME  ("pumpd.pid") 
#define PID_PATH  (__pid_path())
/**
 * __pid_path
 * ``````````
 * Returns the path of the pid file.
 *
 * NOTES
 * In order to signal and stat the daemon, a client needs to know 
 * the pid (process id) of the daemon process. When the daemon is
 * started, it writes this number to the pid file.
 *
 * This is an implementation function that is called through the 
 * PID_PATH macro defined above.
 */
static inline const char *__pid_path(void)
{
        static char buf[PATHSIZE];
        if (!HOME) HOME = sdup(gethome());
        snprintf(buf, PATHSIZE, "%s/%s/%s", HOME, CFG_NAME, PID_NAME);
        return buf;
}


#define CHANNEL(stem) (__channel_path(stem))
#define CH(stem)      (CHANNEL(stem))
/**
 * __channel_path
 * ``````````````
 * Returns the path of the channel identified by 'stem' 
 * 
 * @stem: identifier supplied by the pump daemon.
 *
 * NOTES
 * This is an implementation function that is called through the 
 * CHANNEL or CH macros defined above.
 */
static inline const char *__channel_path(const char *stem)
{
        static char buf[PATHSIZE];
        if (!HOME) HOME = sdup(gethome());
        snprintf(buf, PATHSIZE, "%s/%s/%s", HOME, CFG_NAME, stem);
        return buf;
}


#endif


