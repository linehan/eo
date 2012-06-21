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

#define CFG_PERMS ((S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))

#define FIFO_PUB "fifo.pub"
#define FIFO_SUB "fifo.sub"
#define PID_FILE "pumpd.pid"
#define CFG_STEM ".pump"

#define CFG_PATH (CONCAT((homedir((getuid()))), "/"CFG_STEM"/"CFG_STEM))
#define PID_PATH (CONCAT((homedir((getuid()))), "/"CFG_STEM"/"PID_FILE))
#define PUB_PATH (CONCAT((homedir((getuid()))), "/"CFG_STEM"/"FIFO_PUB))
#define SUB_PATH (CONCAT((homedir((getuid()))), "/"CFG_STEM"/"FIFO_SUB))

#define PERMS 0666

/**
 * do_pump -- apply the logic across each file of the directory
 */
/*void do_pump(void)*/
/*{*/
        /*DIR *dir;*/
        /*char *filename;*/
        /*char execute[512];*/

        /*if (!exists(ENV.logic))*/
                /*bye("Pump contains no logic.");*/

        /*dir = opendir("./");*/

        /*for (filename  = getfile(dir, F_REG);*/
             /*filename != NULL;*/
             /*filename  = getfile(NULL, F_REG))*/
        /*{*/
                /*sprintf(execute, "./%s %s", ENV.logic, filename);*/
                /*system(execute);*/
        /*}*/

        /*closedir(dir);*/
/*}*/



/**
 * usage -- print the usage statement
 */
void usage(void)
{
        printf("%s\n", USAGE_MESSAGE);
}


/**
 * pumpd -- the function executed by the running daemon
 * @dpx: pointer to an open duplex stream
 */
void pumpd(struct dpx_t *dpx)
{
        for (;;) {
                memset(dpx->buf, '\0', MIN_PIPESIZE);
                read_dpx(dpx);
                if (dpx->buf[0] != '\0') {
                        write_dpx(dpx);
                }
        }
        close_dpx(dpx);
}


/**
 * pumpd_start -- start the pump daemon
 */
void pumpd_start(void)
{
        struct dpx_t dpx;

        umask(0); /* Reset process permissions mask */ 

        if (!exists(CFG_PATH))
                mkdir(CFG_PATH, CFG_PERMS);

        new_fifo(SUB_PATH, PERMS);
        new_fifo(PUB_PATH, PERMS);

        daemonize(); 

        // ---------- process is now a daemon ---------- */

        pidfile(PID_PATH, "w+");

        dpx.role = PUBLISH;
        open_dpx(&dpx, PUB_PATH, SUB_PATH);

        pumpd(&dpx);
}


/**
 * pumpd_stop -- stop the pump daemon
 */
void pumpd_stop(void)
{
        int pid;

        if (pid = pidfile(PID_PATH, "r"), !pid)
                bye("pumpd is not running.");

        remove(PID_PATH);
        unlink(SUB_PATH);
        unlink(PUB_PATH);

        kill(pid, SIGTERM);
}


/**
 * pumpd_stat -- stat the pump daemon
 */
void pumpd_stat(void)
{
        int pid;
        if (pid = pidfile(PID_PATH, "r"), !pid)
                bye("pumpd is not running.");
        else    
                bye("pumpd is running with pid %d.", pid);
}





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


