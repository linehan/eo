#define __MERSENNE__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

#include "suc.h"
#include "meta.h"
#include "parse.h"
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
 ******************************************************************************/
int bigpid;

void catchsig(int signo)
{
        if (bigpid != 0)
                kill(bigpid, SIGUSR1);

        signal(signo, SIG_DFL);
        raise(signo);
}

/******************************************************************************
 * SUC 
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

        /* Register the pump's PID so we can kill it on hangup. */  
        bigpid = dpx.remote_pid;

        /* Verify path with pump driver */
        dpx_read(&dpx);

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
 * suc_parse
 * `````````
 * Parse the arguments given to the suc invocation and run the program.
 *
 * @argc : the number of arguments received
 * @argv : the arguments received
 * Return: nothing
 */
void suc_parse(int argc, char *argv[])
{
        struct routine_t *r;
        char *filename;

        r = parse(argc, argv);

        while ((r->op[0]->op(r->op[0], &filename)) != -1)
                r->op[1]->op(r->op[1], &filename);
}



/******************************************************************************
 * MAIN 
 ******************************************************************************/
int main(int argc, char *argv[]) 
{
        load_env(&ENV);

        if (!ARG(1))
                usage();

        /* Command words */
        else if (isarg(1, "stat"))
                print_config();

        else if (isarg(1, "init"))
                suc_init();

        else if (isarg(2, ":-")) {
                suc_pump(argc, argv);
        }

        else if (isarg(2, "::"))
                suc_pump(argc, argv);

        else if (isarg(2, ":"))
                suc_parse(argc, argv);

        return 0;
}

