#define __MERSENNE__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

#include "eo.h"
#include "meta.h"
#include "parse.h"
#include "regex.h"
#include "common/io/file.h"
#include "common/io/dir.h"
#include "common/ipc/daemon.h"
#include "common/ipc/channel.h"

#include "common/error.h"
#include "common/util.h"
#include "common/configfiles.h"
#include "common/textutils.h"
#include "common/lib/bloom/bloom.h"


/******************************************************************************
 * SIGNAL HANDLING 
 *
 * It is important that the eo client register itself with the signal 
 * handler. Especially in the case when eo is using a sibling co-process
 * to "share the load", any abnormal termination of the suc process will
 * need to be caught, so that the co-process can be notified via SIGUSR1
 * that the eo process has been terminated, and it needs to terminate as
 * well.
 ******************************************************************************/
int EOPID;

void catchsig(int signo)
{
        if (EOPID != 0)
                kill(EOPID, SIGUSR1);

        signal(signo, SIG_DFL);
        raise(signo);
}


/******************************************************************************
 * EO 
 * 
 * The eo client is the user-invoked side of the eo utility. Depending
 * on the arguments specified, it will run in blocking or non-blocking 
 * mode.
 *
 * In blocking mode, eo will be attached to a sibling process that will
 * handle the iteration and the monitoring of the buffer, while the main
 * eo process executes the specified logic on the values yielded by the
 * other process.
 *
 * In non-blocking mode, eo will run in its own process. Once the set of
 * values to be iterated over is exhausted, eo will terminate and return
 * to the shell.
 *
 ******************************************************************************/

/**
 * usage 
 * `````
 * Print the usage message to stdout and exit
 * 
 * Returns nothing.
 */
void usage(void)
{
        printf("%s", USAGE_MESSAGE);
        exit(0);
}


/**
 * eo_init 
 * ```````
 * Initialize a pump in the current working directory.
 * 
 * Returns nothing.
 */
void eo_init(void)
{
        struct pumpconfig_t config;
        unsigned long salt;
        char hex[65];

        if (is_pump(ENV.cwd)) 
                bye("pump exists");

        make_pump(ENV.cwd);

        salt = mt_random();
        sha256gen(hex, &salt);

        slcpy(config.name, curdir(), LINESIZE);
        slcpy(config.desc, "Default description", LINESIZE);
        slcpy(config.base, scwd(), LINESIZE);
        slcpy(config.sha2, hex, LINESIZE);
        slcpy(config.link, "/foo/bar/bazscript.qux", LINESIZE);
        slcpy(config.wait, "0", LINESIZE);

        write_config(&config, ENV.config);
}


/**
 * eo_pump
 * ```````
 * Run eo_process in perpetuity, with a co-process to help.
 *
 * @argc : the number of arguments received
 * @argv : the arguments received
 * Return: nothing
 */
void eo_pump(struct routine_t *r)
{
        struct dpx_t dpx = {};

        sigreg(catchsig);

        /* Subscribe to the pump daemon's control channel */
        dpx_open(&dpx, CH("control"), CH_SUB);

        /* Send the directory we want sucked. */
        dpx_ping(&dpx, r->op[0]->operand); 

        /* Close control; open the channel control sent us. */ 
        dpx_close(&dpx);
        dpx_olink(&dpx, CH(dpx.buf), CH_SUB);

        EOPID = dpx.remote_pid; // See "Signal Handling", above.

        /* 
         * Receive filenames until a "DONE" message 
         * is received from the pump 
         */
        for (;;) {
                dpx_read(&dpx);
                if (STRCMP(dpx.buf, "DONE")) {
                        dpx_send(&dpx, "ack"); // Tell pump we received "done"
                        continue;
                }
                char *file = sldup(dpx.buf, PATHSIZE);
                r->op[1]->op(r->op[1]->operand, &file);
                free(file);
        }
        dpx_close(&dpx);
}


/**
 * eo_process
 * ``````````
 * Parse the arguments given to the suc invocation and run the program.
 *
 * @argc : the number of arguments received
 * @argv : the arguments received
 * Return: nothing
 */
void eo_process(struct routine_t *r)
{
        char *filename;
        int i;



        while (r->op[0]->op(r->op[0]->operand, &filename), filename) {
                for (i=1; i<r->n; i++) {
                        r->op[i]->op(r->op[i]->operand, &filename);
                }
        } 
}


/**
 * eo_prep
 * ```````
 * Validate and prepare the command line arguments for execution.
 *
 * @argc : number of arguments
 * @argv : vector of arguments
 * Return: nothing.
 */
void eo_prep(int argc, char *argv[])
{
        struct routine_t *r;

        if (isarg(1, "stat"))
                print_config();
        else if (isarg(1, "init"))
                eo_init();
        else if (isarg(1, "kleene")) {
                if ((kleene(argv[2], argv[3]))) printf("match\n");
                else                            printf("no match\n");
        }
        else {
                /* If there is only one argument, assume the CWD. */
                r = (argc==2) ? parse(scwd(), argv[1]) 
                              : parse(argv[1], argv[2]);

                eo_process(r);
        }

        return;
}
        
                        
                        


/* MAIN ***********************************************************************/
int main(int argc, char *argv[]) 
{
        load_env(&ENV);

        if (!ARG(1))
                usage();

        eo_prep(argc, argv);

        return 0;
}

