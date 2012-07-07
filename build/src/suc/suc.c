#define __MERSENNE__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

#include "suc.h"
#include "meta.h"
#include "parse.h"
#include "regex.h"
#include "../common/io/file.h"
#include "../common/io/dir.h"
#include "../common/ipc/daemon.h"
#include "../common/ipc/channel.h"

#include "../common/error.h"
#include "../common/util.h"
#include "../common/configfiles.h"
#include "../common/textutils.h"
#include "../common/lib/bloom/bloom.h"


/******************************************************************************
 * SIGNAL HANDLING 
 *
 * It is important that the suc client register itself with the signal 
 * handler. Especially in the case when suc is using a sibling co-process
 * to "share the load", any abnormal termination of the suc process will
 * need to be caught, so that the co-process can be notified via SIGUSR1
 * that the suc process has been terminated, and it needs to terminate as
 * well.
 ******************************************************************************/
int SUCPID;

void catchsig(int signo)
{
        if (SUCPID != 0)
                kill(SUCPID, SIGUSR1);

        signal(signo, SIG_DFL);
        raise(signo);
}


/******************************************************************************
 * SUC 
 * 
 * The suc client is the user-invoked side of the suc utility. Depending
 * on the arguments specified, it will run in blocking or non-blocking 
 * mode.
 *
 * In blocking mode, suc will be attached to a sibling process that will
 * handle the iteration and the monitoring of the buffer, while the main
 * suc process executes the specified logic on the values yielded by the
 * other process.
 *
 * In non-blocking mode, suc will run in its own process. Once the set of
 * values to be iterated over is exhausted, suc will terminate and return
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
 * suc_init 
 * ````````
 * Initialize a pump in the current working directory.
 * 
 * Returns nothing.
 */
void suc_init(void)
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
 * suc_pump
 * ````````
 * Run suc_process in perpetuity, with a co-process to help.
 *
 * @argc : the number of arguments received
 * @argv : the arguments received
 * Return: nothing
 */
void suc_pump(int argc, char *argv[])
{
        struct routine_t *r;
        struct dpx_t dpx = {};

        sigreg(catchsig);

        r = parse(argc, argv);

        /* Subscribe to the pump daemon's control channel */
        dpx_open(&dpx, CH("control"), CH_SUB);

        /* Send the directory we want sucked. */
        dpx_ping(&dpx, r->op[0]->operand); 

        /* Close control; open the channel control sent us. */ 
        dpx_close(&dpx);
        dpx_olink(&dpx, CH(dpx.buf), CH_SUB);

        SUCPID = dpx.remote_pid; // See "Signal Handling", above.

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
                r->op[1]->op(r->op[1], &file);
                free(file);
        }
        dpx_close(&dpx);
}


/**
 * suc_process
 * ```````````
 * Parse the arguments given to the suc invocation and run the program.
 *
 * @argc : the number of arguments received
 * @argv : the arguments received
 * Return: nothing
 */
void suc_process(int argc, char *argv[])
{
        struct routine_t *r;
        char *filename;
        int i;

        r = parse(argc, argv);

        while ((r->op[0]->op(r->op[0], &filename)) != -1) {
                for (i=0; i<r->n; i++) {
                        r->op[i]->op(r->op[i], &filename);
                }
        }
}



/* MAIN ***********************************************************************/
int main(int argc, char *argv[]) 
{
        load_env(&ENV);

        if (!ARG(1))
                usage();

        else if (isarg(1, "stat"))
                print_config();

        else if (isarg(1, "init"))
                suc_init();

        else if (isarg(1, "kleene")) {
                if ((kleene(argv[2], argv[3])))
                        printf("match\n");
                else
                        printf("no match\n");
        }

        else if (isarg(2, ":-"))
                suc_pump(argc, argv);

        else if (isarg(2, "::"))
                suc_pump(argc, argv);

        else if (isarg(2, ":"))
                suc_process(argc, argv);

        return 0;
}

