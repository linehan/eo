
/******************************************************************************
 * PUMP HANDLER OPERATION
 * 
 * The pump daemon is intended to handle marshalling operations for the
 * inputs specified by the pump program. Once these inputs are marshalled,
 * they can be yielded back to the pump client as required. The job of
 * the client is simply to deliver the individual inputs to the pump logic.
 *
 ******************************************************************************/
struct pump_t {
        struct dpx_t dpx;
        struct breadcrumb_t nav;
        char target[PATHSIZE];
        struct pth_t watcher;
        char ch_id[32];
        int mode;
};

#define P_FORK 1
#define P_KEEP 0

struct pump_t *new_pump(char *target, char *channel, int mode)
{
        struct pump_t *new;

        new = calloc(1, sizeof(struct pump_t));

        new->mode = mode;

        strlcpy(new->target, target, PATHSIZE);
        strlcpy(new->ch_id, channel, 32);

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

        dir = opendir(p->target);

        cwd_shift(&p->nav, p->target);

        /* Wait for client to connect to channel */
        dpx_read(&p->dpx);
        (!dir) ? dpx_send(&p->dpx, "shit.") : dpx_send(&p->dpx, p->target);
        dpx_read(&p->dpx);

        rediff:

        /* Write each filename into the channel */
        for (file  = getdiff(dir, F_REG); 
             file != NULL; 
             file  = getdiff(NULL, F_REG)) 
        {
                dpx_send(&p->dpx, file);
        }

        /* Notify when all filenames have been written */
        dpx_send(&p->dpx, "done");

        /* Wait for further instructions */
        dpx_read(&p->dpx);

        if (!(STRCMP(p->dpx.buf, "STOP"))) {
                sleep(1);
                goto rediff;
        }

        closedir(dir);
        dpx_close(&p->dpx);
        dpx_remove(p->dpx.path);

        cwd_revert(&p->nav);

        exit(0);
}



void open_pump(struct pump_t *p)
{
        if ((p->mode == P_FORK)) {
                if ((daemonize()) == -1) // See daemon.c
                        return;
        }

        // ---------- process is now a daemon ---------- */

        /* Open a duplex channel as publisher */
        dpx_open(&p->dpx, CHANNEL(p->ch_id), CH_NEW | CH_PUB);

        /* Enter the loop driver */
        do_pump(p);
}
