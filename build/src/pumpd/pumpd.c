#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

#include "pumpd.h"
#include "../common/error.h"
#include "../common/daemon.h"
#include "../common/file.h"

#define HOME_PERMS ((S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
#define HOME     "/home/linehan/.pump"
#define FIFO_PUB "fifo.pub"
#define FIFO_SUB "fifo.sub"
#define PID_FILE "pumpd.pid"

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
 */
void pumpd(struct dpx_t *dpx)
{
        char *msg="FIFO is empty.\n";
        static char buffer[1024];

        for (;;) {
                memset(buffer, '\0', 1024);
                fifo_read(dpx->fd_sub, buffer, 1023);
                if (buffer[0] != '\0') {
                        fifo_write(dpx->fd_pub, buffer, strlen(buffer));
                }
        }
        close_dpx(dpx);
}

#define PERMS 0666


/**
 * pumpd_start -- start the pump daemon
 */
void pumpd_start(void)
{
        struct dpx_t dpx;

        umask(0); /* Reset process permissions mask */ 

        if (!exists(HOME))
                mkdir(HOME, HOME_PERMS);

        new_fifo(HOME"/"FIFO_SUB, PERMS);
        new_fifo(HOME"/"FIFO_PUB, PERMS);

        daemonize(); 

        // ---------- process is now daemon ---------- */

        pidfile(HOME"/"PID_FILE, "w+");

        dpx.role = PUBLISH;
        open_dpx(&dpx, HOME"/"FIFO_PUB, HOME"/"FIFO_SUB);

        pumpd(&dpx);
}


/**
 * pumpd_stop -- stop the pump daemon
 */
void pumpd_stop(void)
{
        int pid;

        if (pid = pidfile(HOME"/"PID_FILE, "r"), !pid)
                bye("pumpd is not running.");

        remove(HOME"/"PID_FILE);
        unlink(HOME"/"FIFO_SUB);
        unlink(HOME"/"FIFO_PUB);
        kill(pid, SIGTERM);
}


/**
 * pumpd_stat -- stat the pump daemon
 */
void pumpd_stat(void)
{
        int pid;
        if (pid = pidfile(HOME"/"PID_FILE, "r"), !pid)
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


