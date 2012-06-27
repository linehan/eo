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
 * PUMP HANDLERS
 * 
 * The pump daemon is intended to handle marshalling operations for the
 * inputs specified by the pump program. Once these inputs are marshalled,
 * they can be yielded back to the pump client as required. The job of
 * the client is simply to deliver the individual inputs to the pump logic.
 *
 ******************************************************************************/

struct pump_t {
        struct dpx_t dpx;
        struct pth_t watcher;
        struct nav_t breadcrumb;
        char target[PATHSIZE];
        char channel[PATHSIZE];
        int mode;
};


/**
 * new_pump
 */
struct pump_t *new_pump(char *target, char *channel, int mode)
{
        struct pump_t *new;

        new = calloc(1, sizeof(struct pump_t));

        new->mode = mode;

        strlcpy(new->target, target, PATHSIZE);
        strlcpy(new->channel, channel, 32);

        return new;
}


/**
 * do_pump -- Marshal the inputs specified by the pump client 
 * @dpx : pointer to a duplex channel
 * @path: directory to be marshalled
 * Returns nothing.
 */ 
void do_pump(struct pump_t *p)
{
        const char *file;
        DIR        *dir;

        /* Open directory stream */
        dir = opendir(p->target);

        /* Shift working directory to target */
        nav_shift(&p->breadcrumb, p->target);

        /* Wait for the client to connect to channel */
        dpx_read(&p->dpx);
        dpx_send(&p->dpx, p->target); // verification
        dpx_read(&p->dpx);            // wait for ack

        rediff:

        /* Write each filename into the channel */
        for (file  = getdiff(dir,  F_REG); 
             file != NULL; 
             file  = getdiff(NULL, F_REG)) 
        {
                dpx_send(&p->dpx, file);
        }

        /* 
         * Notify client when all filenames 
         * have been written 
         */
        dpx_send(&p->dpx, "DONE");

        /* 
         * Wait for further instructions from 
         * the client
         */
        dpx_read(&p->dpx);

        /*
         * Unless the client indicates that
         * the pump should be suspended, wait
         * one second and then jump back and
         * begin processing the directory again.
         */
        if (!(STRCMP(p->dpx.buf, "STOP"))) {
                sleep(1);
                goto rediff;
        }

        /*
         * Tidy up any descriptors and/or files 
         * on disk.
         */
        closedir(dir);
        dpx_close(&p->dpx);
        dpx_remove(p->dpx.path);

        /* Restore current working directory */
        nav_revert(&p->breadcrumb);

        exit(0);
}



void open_pump(struct pump_t *p)
{
        if ((p->mode == P_FORK) && ((daemonize()) == -1))
                return;

        // ---------- process is now a daemon ---------- */

        /* Open a new duplex channel as publisher */
        dpx_open(&p->dpx, CHANNEL(p->channel), CH_NEW | CH_PUB);

        /* Enter the loop driver */
        do_pump(p);
}

