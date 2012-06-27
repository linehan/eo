#define __MERSENNE__
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>

#include "pumpd.h"
#include "pumps.h"

#include "../common/error.h"
#include "../common/daemon.h"
#include "../common/file.h"
#include "../common/channel.h"
#include "../common/configfiles.h"
#include "../common/textutils.h"
#include "../common/dir.h"
#include "../common/lib/pth/thread.h"


/******************************************************************************
 * PUMPS 
 * 
 * The pump daemon is intended to handle marshalling operations for the
 * inputs specified by the pump client. The way it does this is by creating
 * a process context in the form of a pump object, and then forking a new
 * process to handle the client's request. The client is then directed to
 * re-connect with the pump object on a dedicated channel.
 *
 * Once the client is talking directly to the pump object, it essentially has
 * its own worker process at its disposal. In the most common usage, the
 * pump object will marshal inputs in the form of a directory listing, and
 * yield those inputs to the client as requested. 
 *
 * In this way, the client must only specify the general logic for handling 
 * a single input, and can leave the marshalling and juggling to the pump 
 * process. 
 *
 ******************************************************************************/

/*
 * The pump object datatype (PRIVATE)
 */

struct pump_t {
        int mode;                // Used by pump daemon
        struct nav_t breadcrumb; // Tracks working directory
        struct dpx_t dpx;        // Duplex channel link
        struct pth_t watcher;    // Watcher thread for DIR I/O 
        char target[PATHSIZE];   // Directory to be pumped
        char channel[PATHSIZE];  // Channel identification string
        DIR *dir;                // Where the money comes out
        int  dir_fd;             // File descriptor version of above
};


/*
 * NOTICE 
 * Because each process has its own stack, and a pump object maps
 * reasonably well to an individual process, the global variable 
 * "current_pump" is not as global as it seems. We need it to have
 * global scope within a process context so that the signal handler
 * can feed it to kill_pump() before the process terminates.
 */
struct pump_t *current_pump;


/**
 * catch_signal
 * ````````````
 * Call kill_pump() to perform cleanup before process termination.
 *
 * @signo : signal number sent from the kernel
 * Returns: does not return.
 */
void catch_signal(int signo)
{
        if (current_pump)
                kill_pump(current_pump);

        signal(signo, SIG_DFL);
        raise(signo);
}


/**
 * register_pump
 * `````````````
 * Register the pump object and set the signal handlers.
 *
 * @p: pump object
 * Returns: nothing.
 */
void register_pump(struct pump_t *p)
{
        current_pump = p;
        sigreg(catch_signal);
}


/**
 * new_pump
 * ````````
 * Create a dynamically-allocated pump object.
 * 
 * @target : the directory to be pumped
 * @channel: the name of the channel to publish on
 * @mode   : whether to fork the process or not
 * Return  : pointer to an allocated pump object.
 *
 */
struct pump_t *new_pump(char *target, char *channel, int mode)
{
        struct pump_t *new;

        new = calloc(1, sizeof(struct pump_t));

        new->mode = mode;

        strlcpy(new->target, target, PATHSIZE);
        strlcpy(new->channel, channel, PATHSIZE);

        return new;
}


/**
 * kill_pump
 * `````````
 * Perform cleanup and arrange for an orderly termination of the process.
 *
 * Return: does not return.
 */
void kill_pump(struct pump_t *p)
{
        /* Close directory stream if it's open */
        if (p->dir)
                closedir(p->dir);

        /* Close and unlink files on disk */
        dpx_close(&p->dpx);
        dpx_remove(p->dpx.path);

        /* Restore current working directory */
        nav_revert(&p->breadcrumb);
}


/**
 * open_pump 
 * `````````
 * Open a pump and its associated IPC channel, fork if mode specifies.
 *
 * @p    : pointer to an un-initialized pump object 
 * Return: nothing / child does not return.
 *
 * NOTES 
 * After a pump has been "opened", it is essentially on its own, off
 * in whatever elysian memory the kernel gives it. After this call,
 * responsibility for the transaction effectively transfers to the 
 * client. 
 */ 
void open_pump(struct pump_t *p)
{
        if ((p->mode == P_FORK) && ((daemonize()) == -1))
                return;

        // ---------- process is now a daemon ---------- */

        register_pump(p);

        /* Open a new duplex channel as publisher */
        dpx_open(&p->dpx, CHANNEL(p->channel), CH_NEW | CH_PUB);

        /* Enter the loop driver */
        pump_files(p);
}


/**
 * pump_idle
 * `````````
 * Listen on the open directory file descriptor and nanosleep.
 *
 * @p     : pointer to a running pump object
 * @ns    : number of nanoseconds to sleep between stat calls
 * Returns: When the directory has been modified
 *
 * NOTES
 * The idle loop is pretty simple. The directory's file descriptor
 * is queried using the fstat command, and the st_mtime member of
 * the stat struct is examined. If its value is less than the time
 * recorded when this function was called, the directory contents
 * have not been modified, and we can continue idling. 
 *
 * USAGE 
 * In practice, long nanoseconds may be hard to find. 
 */
void pump_idle(struct pump_t *p, long nanoseconds)
{
        time_t at_start;
        struct stat buf;

        at_start = time(NULL);

        /* 
         * If the file descriptor is rotten, return 
         * immediately (no sleeping).
         */
        if (fstat(p->dir_fd, &buf) == EBADF)
                return;

        do { 
                fstat(p->dir_fd, &buf);
                nsleep(nanoseconds);

        } while (buf.st_mtime <= at_start);
}



/**
 * pump_files 
 * ``````````
 * Transmit filenames in the target directory to the client.
 *
 * @p    : pointer to an initialized pump object 
 * Return: does not return.
 *
 * NOTES
 * Once this function is entered, the pump object and the worker thread
 * associated with it will only exist for the duration of the client's
 * connection. If a disconnection occurs, the channel and memory will
 * be free'd and the process will be killed. 
 */ 
void pump_files(struct pump_t *p)
{
        const char *file;

        /* Open directory stream */
        p->dir     = opendir(p->target);
        p->dir_fd  = dirfd(p->dir);

        /* Shift working directory to target */
        nav_shift(&p->breadcrumb, p->target);

        /* Wait for the client to connect to channel */
        dpx_link(&p->dpx);
        dpx_send(&p->dpx, p->target); // verify target 

        rediff:

        /* 
         * Write each new filename into the channel. 
         * Filenames that have already been sent are
         * not yielded by getdiff, which keeps a Bloom
         * filter to screen out the double-dippers. 
         */
        while ((file = getdiff(p->dir, F_REG))) {
                dpx_send(&p->dpx, file);
        }

        dpx_send(&p->dpx, "DONE"); // notify client
        dpx_read(&p->dpx);         // wait for instructions

        /*
         * Idle the pump until the directory contents 
         * are modified.
         */
        pump_idle(p, 100);
        goto rediff;

        exit(0);
}


