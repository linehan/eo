#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ipc.h>

#include "../io/file.h"
#include "../error.h"
#include "../textutils.h"
#include "daemon.h"


#define PERMS (0666)


/******************************************************************************
 * PID MANAGEMENT
 * 
 * Once a process has been daemonized and is executing in the background,
 * client processes need some means of getting in touch with the daemon.
 * The traditional kludge is for a daemon to leave behind a "pidfile" at
 * some predetermined location, soon after forking into the background.
 *
 * A pidfile is, unsurprisingly, a small file containing the pid (process
 * id) of the daemon. The daemon should create one soon after it begins to
 * execute, and delete the file before it terminates.
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
                     : bye("daemon: No pidfile. Is the daemon running?");
        }
        else
                bye("daemon: Invalid mode supplied to pidfile().");
        
        fclose(fp);
        return pid;
}



/******************************************************************************
 * DAEMONIZE 
 ******************************************************************************/

/** 
 * daemonize -- fork the current process and prepare it to operate as a daemon
 */
int daemonize(void)
{
        int i;
        /* 
         * The child will continue executing daemonize(), while
         * the parent returns 0 to the shell.
         */
	if (fork() != 0)
                return -1;

        for (i=0; i<NOFILE; i++) /* Close files inherited from parent */
                close(i);

        umask(0);                /* Reset file access creation mask */
	signal(SIGCLD, SIG_IGN); /* Ignore child death */
	signal(SIGHUP, SIG_IGN); /* Ignore terminal hangups */
	setpgrp();               /* Create new process group */

        return 0;
}


