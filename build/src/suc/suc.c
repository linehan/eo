#define __MERSENNE__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

#include "suc.h"
#include "meta.h"
#include "parse.h"
#include "../common/io/file.h"
#include "../common/ipc/daemon.h"
#include "../common/ipc/channel.h"

#include "../common/error.h"
#include "../common/util.h"
#include "../common/configfiles.h"
#include "../common/textutils.h"
#include "../common/lib/bloom/bloom.h"


static char LOGIC[20000];


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
 * pump_init 
 * `````````
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


void suc_list(char *path)
{
        struct dpx_t dpx;
        char abspath[PATHSIZE];

        sigreg(catchsig);

        slcpy(abspath, path, PATHSIZE);
        make_path_absolute(abspath);

        /* Subscribe to the pump daemon's control channel */
        dpx_open(&dpx, CH("control"), CH_SUB);

        dpx_send(&dpx, abspath); // Send the path we want to be listed
        dpx_read(&dpx);          // Wait for response... 

        /* (diagnostic) */
        printf("targ: %s\n", abspath);
        printf("name: %s\nchan: %s\n\n", dpx.buf, CH(dpx.buf));

        /* 
         * Close the control channel and open 
         * the channel that control sent us.
         */ 
        dpx_close(&dpx);
        dpx_open(&dpx, CH(dpx.buf), CH_SUB);
        dpx_link(&dpx);

        /*
         * Print the pump server's PID (diagnostic) and register
         * it with the signal handler so we can tell it we hung up.
         */
        printf("pump PID: %d\n", dpx.remote_pid);
        bigpid = dpx.remote_pid;

        /* 
         * Verify path with pump driver 
         */
        dpx_read(&dpx);
        printf("pump reports path: %s\n", dpx.buf);

        /* 
         * Receive file listing until "done" message 
         * is received from the pump 
         */
        for (;;) {
                dpx_read(&dpx);
                if (STRCMP(dpx.buf, "DONE")) {
                        dpx_send(&dpx, "ack"); // Tell pump we received "done"
                        continue;
                }
                printf("%s\n", dpx.buf);
        }
        dpx_close(&dpx);
}


/**
 * pump_print
 * ``````````
 * Print the filenames in the directory at path to stdout.
 * 
 * Returns nothing.
 */ 
void suc_print(char *path)
{
        const char *filename;
        DIR *dir;

        dir = sdopen(path);

        while ((filename = getfile(dir, F_REG))) {
                printf("%s\n", filename);
        }
        sdclose(dir);

        return;
}


/**
 * suc_logic
 * `````````
 * Apply logic in .suc to each filename yielded by the iterator.
 * 
 * Returns nothing.
 */
void suc_logic(char *path)
{
        struct pumpconfig_t config;
        const char *filename;
        DIR *dir;
        static char buf[3000];

        if (!is_pump(path))
                bye("Current directory does not appear to be a pump.\n");

        read_config(&config, "./.pump/config");

        dir = sdopen(path);

        while ((filename = getfile(dir, F_REG))) {
                snprintf(buf, 3000, "./.pump/%s \"%s\" \"%s\"", config.link, config.base, filename);
                printf("%s\n", buf);
                system(buf);
        }
        sdclose(dir);
}



void catenate(char *dest, size_t len, int argc, char *argv[])
{
        int i;

        for (i=0; i<argc; i++) {
                slcat(dest, argv[i], len);
                slcat(dest, " ", len);
        }
}


void suc_parse(int argc, char *argv[])
{
        struct routine_t *r;
        char buf[LINESIZE];
        int i;

        catenate(buf, LINESIZE, argc, argv); 
        r = parse(buf);

        for (i=0; i<r->n; i++) {
                printf("operation %d: %s\n", i, symbol_text[r->op[i]->tag]);
                printf("command: %s\n", r->op[i]->command);
        }


        char *filename;
        DIR *dir;
        char file[PATHSIZE];
        char path[PATHSIZE];

        slcpy(path, r->op[0]->command, PATHSIZE);
        make_path_absolute(path);

        dir = sdopen(path);

        while ((filename = getfile(dir, F_REG))) {
                slcpy(file, filename, PATHSIZE);
                printf("before: %s\n", file);
                for (i=0; i<r->n; i++) {
                        r->op[i]->op(r->op[i], file);
                }
                printf("after: %s\n\n", file);
        }
        sdclose(dir);
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

        /*else if (isarg(1, "parse"))*/
                /*suc_parse(argc, argv);*/

        /*else if (isarg(1, "at"))*/
                /*suc_at();*/

        /* Default if no operator specified */
        else if (!ARG(2))
                suc_print(ARG(1));

        /* Operators */
        else if (isarg(2, ":L"))
                suc_logic(ARG(1));

        else if (isarg(2, ":-"))
                suc_list(ARG(1));

        else if (isarg(2, ":"))
                suc_parse(argc, argv);

        return 0;
}

