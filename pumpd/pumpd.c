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

#define PIDDIR "/home/linehan/src/mine/pump/pumpd/pumpd.pid"
#define FIFO_READ  "/home/linehan/src/mine/pump/pumpd/pumpd.read"
#define FIFO_WRITE "/home/linehan/src/mine/pump/pumpd/pumpd.write"

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
void pumpd(int read, int write)
{
        char *msg="FIFO is empty.\n";
        static char buffer[1024];
        int safety;

        safety = open_fifo(FIFO_READ, 'w');

        for (;;) {
                /*fifo_write(write, msg, strlen(msg));*/
                fifo_read(read, buffer, 1024);
                if (buffer[0] != '\0') {
                        fifo_write(write, buffer, strlen(buffer));
                        memset(buffer, '\0', strlen(buffer));
                }
        }
        close_fifo(write);
        close_fifo(read);
}

#define PERMS 0666

/**
 * pumpd_start -- start the pump daemon
 */
void pumpd_start(void)
{
        int read;
        int write;

        umask(0); /* Reset process permissions mask */ 

        new_fifo(FIFO_READ, PERMS);
        new_fifo(FIFO_WRITE, PERMS);

        daemonize(); 

        // ---------- process is now daemon ---------- */

        pidfile(PIDDIR, "w+");

        read  = open_fifo(FIFO_READ,  'r');
        write = open_fifo(FIFO_WRITE, 'w');

        pumpd(read, write);
}


/**
 * pumpd_stop -- stop the pump daemon
 */
void pumpd_stop(void)
{
        int pid;
        if (pid = pidfile(PIDDIR, "r"), !pid)
                bye("pumpd is not running.");
        remove(PIDDIR);
        unlink(FIFO_READ);
        unlink(FIFO_WRITE);
        kill(pid, SIGTERM);
}


/**
 * pumpd_stat -- stat the pump daemon
 */
void pumpd_stat(void)
{
        int pid;
        if (pid = pidfile(PIDDIR, "r"), !pid)
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


