#define __MERSENNE__
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "pumpd.h"
#include "../common/error.h"
#include "../common/daemon.h"
#include "../common/file.h"
#include "../common/channel.h"
#include "../common/configfiles.h"
#include "../common/textutils.h"
#include "../common/dir.h"
#include "../common/lib/pth/thread.h"



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
 * usage -- Print the usage statement to stdout
 * Returns nothing.
 */
void usage(void)
{
        printf("%s\n", USAGE_MESSAGE);
}


/**
 * pumpd -- The loop driver executed by the running daemon
 * @dpx: pointer to an open duplex stream in PUBLISH mode
 * Returns nothing.
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
                        /*spawn_pump_handler(dpx->buf, id);*/
                        struct pump_t *p = new_pump(dpx->buf, id, P_FORK);
                        open_pump(p);

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
 * pumpd_init -- Perform filesystem checks and prepare environment
 * Returns nothing.
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
 * pumpd_start -- Spawn a daemon, acquire a duplex channel, and listen
 * Returns nothing.
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
 * pumpd_stop -- Terminate the daemon and perform cleanup
 * Returns nothing.
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
 * pumpd_stat -- Print the status of the pump daemon to stdout
 * Returns nothing.
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

