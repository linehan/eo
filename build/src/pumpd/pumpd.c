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


/******************************************************************************
 * PUMPD CONTROL 
 * 
 * The pump daemon is fairly simple at this point. When started, it
 * initializes several files and then enters a loop driver, where it
 * remains until it receives a SIGTERM, hopefully from pumpd_stop().  
 *
 * FIXME: Transplant the signal handler from Barnicle into error.c
 *        in order to perform cleanup after unexpected SIGTERMs.
 *
 * pumpd_init    # Create initial files
 * pumpd_start   # Start the daemon 
 * pumpd_stop    # Stop the daemon 
 * pumpd_stat    # Return the status of the daemon
 *
 ******************************************************************************/

/**
 * usage 
 * `````
 * Print the usage statement to stdout
 * 
 * Return: nothing.
 */
void usage(void)
{
        printf("%s\n", USAGE_MESSAGE);
}


/**
 * pumpd_init 
 * ``````````
 * Perform filesystem checks and prepare environment.
 * 
 * Return: nothing.
 */
void pumpd_init(void)
{
        /* Reset permissions mask */
        umask(0); 

        /* Create configuration directory if none present */
        if (!exists(CFG_PATH))
                mkdir(CFG_PATH, DIR_PERMS);
}


/**
 * pumpd
 * `````
 * The loop driver executed by the running daemon
 * 
 * @dpx  : pointer to an open duplex stream in PUBLISH mode
 * Return: does not return.
 *
 * NOTES
 * This is where the daemon really spends most of its time, the closest
 * thing to a "main loop" that exists at the moment. It basically waits
 * for a client to provide it with a path, then it creates a pump to
 * handle that path and tells the client and pump what channel to meet
 * up on. 
 *
 */
void pumpd(struct dpx_t *dpx)
{
        char id[PATHSIZE];

        for (;;) {
                /* Load the tempfile template */
                strlcpy(id, "XXXXXX", PATHSIZE);

                /* Wait for a pump request */
                dpx_read(dpx);

                if (dpx->buf[0] != '\0') {
                        /* Turn the template into a tempname */
                        tempname(id);

                        /* Spawn a pump handler with that name */
                        open_pump(new_pump(dpx->buf, id, P_FORK));

                        /* Wait for pump handler to create files */
                        while (!exists(CHANNEL(id)))
                                ;
                        /* Tell client where to connect */
                        dpx_send(dpx, id);
                }
        }
        dpx_close(dpx);
}


/**
 * pumpd_start 
 * ```````````
 * Spawn a daemon, acquire a duplex channel, and listen
 *
 * Return: does not return.
 *
 * FIXME
 * This needs to be factored with pump_open() in pumps.c 
 *
 */
void pumpd_start(void)
{
        struct dpx_t dpx;

        pumpd_init();

        if ((daemonize()) == -1) // See daemon.c
                return;

        // ---------- process is now a daemon ---------- */

        /* Create a new pidfile */
        pidfile(PID_PATH, "w+");

        /* Open the control channel as publisher */
        dpx_open(&dpx, CHANNEL("control"), CH_NEW | CH_PUB);

        /* Enter the loop driver */
        pumpd(&dpx);
}


/**
 * pumpd_stop 
 * ``````````
 * Terminate the daemon and perform cleanup
 *
 * Return: does not return.
 */
void pumpd_stop(void)
{
        int pid;

        if (pid = pidfile(PID_PATH, "r"), !pid)
                bye("pumpd is not running.");

        remove(PID_PATH);
        dpx_remove(CHANNEL("control"));

        kill(pid, SIGTERM);
}


/**
 * pumpd_stat 
 * ``````````
 * Print the status of the pump daemon to stdout
 *
 * Return: nothing.
 */
void pumpd_stat(void)
{
        int pid;
        if (pid = pidfile(PID_PATH, "r"), !pid)
                bye("pumpd is not running.");
        else    
                bye("pumpd is running with pid %d.", pid);
}



/******************************************************************************
 * MAIN
 ******************************************************************************/
int main(int argc, char *argv[])
{
        if (argc == 1)
                usage();

        else if (isarg(1, "start"))
                pumpd_start();

        else if (isarg(1, "stop"))
                pumpd_stop();

        else if (isarg(1, "stat"))
                pumpd_stat();

        else if (isarg(1, "help") || isarg(1, "?"))
                usage();

        return 0;
}

