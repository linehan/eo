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

#include "../common/error.h"

#define PIDDIR "/home/linehan/src/mine/pump/pumpd/pumpd.pid"

/**
 * write the pid to the pid file
 */
void writepid(const char *path)
{
        FILE *fp;

        if (fp = fopen(path, "w+"), fp == NULL)
                bye("pid file exists");

        fprintf(fp, "%d", getpid());
        fclose(fp);
}


/**
 * read the pid from the pid file (destroys pid file)
 */
int readpid(const char *path)
{
        FILE *fp;
        int pid;

        if (fp = fopen(path, "r"), fp == NULL)
                return 0;

        fscanf(fp, "%d", &pid);
        fclose(fp);
        
        return pid;
}


/** 
 * daemonize -- fork the current process and prepare it to operate as a daemon
 */
void daemonize(void)
{
        int i;
        /* 
         * The child will continue executing daemonize(), while
         * the parent returns 0 to the shell.
         */
	if (fork() != 0)
                exit(0);

        for (i=0; i<NOFILE; i++) /* Close files inherited from parent */
                close(i);

        umask(0);                /* Reset file access creation mask */
	signal(SIGCLD, SIG_IGN); /* Ignore child death */
	signal(SIGHUP, SIG_IGN); /* Ignore terminal hangups */
	setpgrp();               /* Create new process group */
}


/**
 * do_pump -- apply the logic across each file of the directory
 */
void do_pump(void)
{
        DIR *dir;
        char *filename;
        char execute[512];

        if (!exists(ENV.logic))
                bye("Pump contains no logic.");

        dir = opendir("./");

        for (filename  = getfile(dir, F_REG);
             filename != NULL;
             filename  = getfile(NULL, F_REG))
        {
                sprintf(execute, "./%s %s", ENV.logic, filename);
                system(execute);
        }

        closedir(dir);
}


/**
 * pumpd -- the function executed by the running daemon
 */
void pumpd(void)
{
        for (;;) {
                sleep(300);
        }
}


/**
 * pumpd_start -- start the pump daemon
 */
void pumpd_start(void)
{
        daemonize();
        writepid(PIDDIR);
        pumpd();
}


/**
 * pumpd_stop -- stop the pump daemon
 */
void pumpd_stop(void)
{
        int pid;
        if (pid = readpid(PIDDIR), !pid)
                bye("daemon is not running.");
        remove(PIDDIR);
        kill(pid, SIGTERM);
}


/**
 * pumpd_stat -- stat the pump daemon
 */
void pumpd_stat(void)
{
        int pid;
        if (pid = readpid(PIDDIR), !pid)
                bye("daemon is not running.");
        else
                bye("daemon is running with pid %d.", pid);
}


/**
 * pumpd_usage -- print the usage statement 
 */
void pumpd_help(void)
{
        printf("The pump daemon manages the pumps registered in the filesystem\n\n" 
               "Usage: pumpd <DIRECTIVE>\n\n"
               "\tstart   - start the pump daemon\n"
               "\tstop    - stop the pump daemon if it is running\n"
               "\trestart - stop and then start the pump daemon\n"
               "\tstat    - report the daemon's status\n"
               "\thelp    - print this message\n"
               "\n");
}


int main(int argc, char *argv[])
{
        if (argc == 1)
                pumpd_help();

        else if (isarg(1, "start"))
                pumpd_start();

        else if (isarg(1, "stop"))
                pumpd_stop();

        else if (isarg(1, "stat"))
                pumpd_stat();

        else if (isarg(1, "help") || isarg(1, "?"))
                pumpd_help();

        return 0;
}


