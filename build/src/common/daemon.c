#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <limits.h>

#include "error.h"
#include "file.h"
#include "textutils.h"
#include "daemon.h"

/******************************************************************************
 * PID MANAGEMENT
 ******************************************************************************/

/**
 * pidfile - read or write a pidfile for the current process
 * @path: path to the pidfile
 * @mode: mode to open the pidfile with (influences return value) 
 */
int pidfile(const char *path, const char *mode)
{
        FILE *fp;
        int pid;

        fp  = fopen(path, mode);
        pid = getpid();

        if (*mode == 'w') {
                (fp) ? fprintf(fp, "%d", pid) 
                     : bye("daemon: Cannot open pidfile for writing.");
        } 
        else if (*mode == 'r') {
                (fp) ? fscanf(fp, "%d", &pid)
                     : bye("daemon: Cannot read pidfile. Is daemon running?");
        }
        else
                bye("daemon: Invalid mode supplied to pidfile().");
        
        fclose(fp);
        return pid;
}


/******************************************************************************
 * IPC 
 *
 *      "The daemon opens the FIFO for read-only and its typical state 
 *       is waiting in a 'read' system call for a client request. 
 *
 *       Client processes are started and they open the FIFO for writing, 
 *       write their request, and exit. What happens is the 'read' 
 *       returns zero (end of file) to the daemon every time a client 
 *       process terminates, if no other clients have the FIFO open for 
 *       writing. 
 *
 *       The daemon has to then 'open' the FIFO again (for read-only) 
 *       and it waits here until a client process 'open's the 
 *       FIFO for writing. 
 *
 *       To avoid this, a useful technique is for the daemon to open the 
 *       FIFO two times -- once for reading and once for writing. The file 
 *       descriptor returned for reading is used to 'read' the client 
 *       requests, and the file descriptor for writing is never used. 
 *
 *       By having the FIFO always open for writing (as long as the daemon 
 *       process exists) the reads do not return an EOF, but wait for the 
 *       next client request."
 * 
 *                                              Stevens (1990), pp. 112
 *
 ******************************************************************************/
#define PERMS (0666)
/**
 * new_fifo -- create a new fifo for the daemon to listen on
 * @path: path of the FIFO to be created
 * @mode: permissions OR'ed with an option such as 
 */
void new_fifo(const char *path, int permissions)
{
        if ((mknod(path, S_IFIFO | permissions, 0)) == -1) {
                bye("Fee! Fie! Foe! Fum! The FIFO failed to open!");
        }
}


/******************************************************************************
 * OPENING AND CLOSING FIFOs 
 *
 *  time  server process                          client process   
 *  ===================================================================
 *   00  [SERVER]     ....                            ....       ....    
 *          ⬇⬇ 
 *   01  [SERVER] ----call--->  open(FIFO1, 'w')      ....       ....   
 *          ⬇⬇ 
 *   02  [SERVER] ----wait----  open(FIFO1, 'w')      ....       ....  
 *          ⬇⬇ 
 *   03  [SERVER] ----wait----  (wait on reader)      ....     [CLIENT]
 *          ⬇⬇                                                    ⬇⬇
 *   04  [SERVER] ----wait----  open(FIFO1, 'r')  <---call---- [CLIENT]
 *          ⬇⬇                                                    ⬇⬇
 *   05  [SERVER] <--WRONLY---  FIFO1 descriptor  ---RDONLY--> [CLIENT]
 *          ⬇⬇                                                    ⬇⬇
 *   06  [SERVER] ---write--->  FIFO1 descriptor  ----read---> [CLIENT]
 *          ⬇⬇                                                    ⬇⬇
 *   07  [SERVER] ----call--->    close(FIFO1)    <---call---- [CLIENT]
 *  ====================================================================
 *
 * The order of the open calls is important, and avoids a deadlock
 * condition. When the server opens FIFO1 for writing, it waits until
 * the client opens it for reading. If the first call to open in the
 * client were for FIFO2 instead of FIFO1, then the client would wait
 * for the parent to open FIFO2 for writing. Each process would then
 * be waiting for the other, and neither would proceed.
 *
 * The solution is to either code the calls to open in the appropriate
 * way, or else specify the O_NDELAY option ('n' in this API) to open().
 *
 *                                              Stevens (1990), pp. 113
 ******************************************************************************/

/**
 * open_fifo -- open a FIFO with the specified mode 
 * @path: path to the already-created FIFO
 * @mode: 'r' or 'w' for read/write. Can be OR'ed with 'n' for non-blocking
 * Returns file descriptor for the FIFO 
 */
int open_fifo(const char *path, const char *mode)
{
        mode_t MODE;
        size_t len;
        int fd;

        len = strlen(mode);

        if ((memchar((void *)mode, 'r', len))) MODE  = O_RDONLY;
        if ((memchar((void *)mode, 'w', len))) MODE  = O_WRONLY;
        if ((memchar((void *)mode, 'n', len))) MODE |= O_NDELAY;

        if ((fd = open(path, MODE)) < 0)
                bye("daemon: could not open fifo");

        /* keepalive -- see note (Stevens) above */
        if ((memchar((void *)mode, 'k', len))) open_fifo(path, "w");

        return fd;
}


/**
 * close_fifo -- close a FIFO file descriptor
 * @fd: file descriptor of the file to be closed
 * Returns nothing 
 */
void close_fifo(int fd)
{
        if ((close(fd)) < 0)
                bye("daemon: could not close fifo");
}
        

/**
 * fifo_write -- write the contents of a buffer into a fifo
 * @fd: file descriptor
 * @buf: buffer to be written to fifo
 * @len: size of the buffer
 * Returns nothing
 */
void fifo_write(int fd, void *buf, size_t len)
{
        if ((write(fd, buf, len)) == -1)
                bye("daemon: could not write to fifo");
}


/**
 * fifo_read -- read the contents of a fifo into a buffer
 * @fd: file descriptor
 * @buf: buffer to be written to fifo
 * @len: size of the buffer
 * Returns nothing
 */
void fifo_read(int fd, void *buf, size_t len)
{
        #define NULterminate
        size_t z;

        if ((z = read(fd, buf, len)) == -1)
                bye("daemon: could not write to fifo");

        #if defined(NULterminate)
        ((char *)buf)[z] = '\0';
        #endif
}


/******************************************************************************
 * CHANNELS 
 ******************************************************************************/

void open_dpx(struct dpx_t *dpx, const char *pub_path, const char *sub_path)
{
        if (dpx->role == PUBLISH) {
                dpx->fd_sub = open_fifo(sub_path, "r");
                dpx->fd_nub = open_fifo(sub_path, "w");
                dpx->fd_pub = open_fifo(pub_path, "w");
        } else if (dpx->role == SUBSCRIBE) {
                dpx->fd_pub = open_fifo(sub_path, "w");
                dpx->fd_sub = open_fifo(pub_path, "r");
        } else {
                bye("open_dpx: Invalid duplex role");
        }
}


void close_dpx(struct dpx_t *dpx)
{
        if (dpx->role == PUBLISH) {
                close(dpx->fd_sub);
                close(dpx->fd_nub);
                close(dpx->fd_pub);
        } else if (dpx->role == SUBSCRIBE) {
                close(dpx->fd_pub);
                close(dpx->fd_sub);
        } else {
                bye("close_dpx: Invalid duplex role");
        }
}


/******************************************************************************
 * DAEMONIZE 
 ******************************************************************************/

/** 
 * daemonize -- fork the current process and prepare it to operate as a daemon
 */
void daemonize(void)
{
        int i;
        /* 
         * The child will continue executing daemonize(), while
         * the parent returns 0 to the shell.
         */
	if (fork() != 0)
                exit(0);

        for (i=0; i<NOFILE; i++) /* Close files inherited from parent */
                close(i);

        umask(0);                /* Reset file access creation mask */
	signal(SIGCLD, SIG_IGN); /* Ignore child death */
	signal(SIGHUP, SIG_IGN); /* Ignore terminal hangups */
	setpgrp();               /* Create new process group */
}


