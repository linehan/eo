#define __MERSENNE__
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>

#include "pumpd.h"
#include "../common/error.h"
#include "../common/daemon.h"
#include "../common/file.h"
#include "../common/channel.h"
#include "../common/configfiles.h"


/******************************************************************************
 * PUMPD OPERATION
 * 
 * The pump daemon is intended to handle marshalling operations for the
 * inputs specified by the pump program. Once these inputs are marshalled,
 * they can be yielded back to the pump client as required. The job of
 * the client is simply to deliver the individual inputs to the pump logic.
 *
 ******************************************************************************/

/**
 * do_pump -- Marshal the inputs specified by the pump client 
 */ 
const char *do_pump(struct dpx_t *dpx, const char *path)
{
        DIR  *dir;
        char *filename;

        dir = opendir(path);

        dpx_read(dpx); // Wait for client to connect

        for (filename  = getfile(dir, F_REG);
             filename != NULL;
             filename  = getfile(NULL, F_REG))
        {
                dpx_load(dpx, filename);
                dpx_write(dpx);
        }
        dpx_read(dpx);

        if (STRCMP(dpx->buf, "done")) {
                dpx_close(dpx);
                closedir(dir);
        }
}


/**
 * spawn_pump_handler -- Create a process to handle a pump client request
 * @id: unique identifier for named pipes and other resolution 
 */
void spawn_pump_handler(const char *path, const char *id)
{
        struct dpx_t dpx;

        dpx_creat(FIFO(id), PERMS);

        if ((daemonize()) == -1) // See daemon.c
                return;

        // ---------- process is now a daemon ---------- */

        /* Open a duplex channel as publisher */
        dpx.role = PUBLISH;
        dpx_open(&dpx, FIFO(id));

        /* Enter the loop driver */
        do_pump(&dpx, path);
}



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

inline char *sha2(void)
{
        unsigned long salt;
        static char hex[65];

        salt = mt_random();
        sha256gen(hex, &salt);

        return hex;
}


/**
 * pumpd -- The loop driver executed by the running daemon
 * @dpx: pointer to an open duplex stream in PUBLISH mode
 * Returns nothing.
 */
void pumpd(struct dpx_t *dpx)
{
        for (;;) {
                memset(dpx->buf, '\0', MIN_PIPESIZE);
                dpx_read(dpx);
                /*if (dpx->buf[0] != '\0') {*/ /* echo */
                        /*dpx_write(dpx);*/
                /*}*/
                if (dpx->buf[0] != '\0')
                        spawn_pump_handler(dpx->buf, "test");

                /* Tell client where to connect */
                dpx_load(dpx, "test");
                dpx_write(dpx);
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

        /* Create named pipes for duplex channel */
        dpx_creat(FIFO_PATH, CFG_PERMS);
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

        /* Open a duplex channel as publisher */
        dpx.role = PUBLISH;
        dpx_open(&dpx, FIFO_PATH);

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
        dpx_remove(FIFO_PATH);

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

