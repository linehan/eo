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


void writepid(const char *path)
{
        FILE *fp;

        if (fp = fopen(path, "w+"), fp == NULL)
                bye("pid file exists");
        else 
                fprintf(fp, "%d", getpid());

        fclose(fp);
}

int readpid(const char *path)
{
        FILE *fp;
        int pid;

        if (fp = fopen(path, "r"), fp == NULL)
                bye("pid file does not exist");
        else
                fscanf(fp, "%d", &pid);

        fclose(fp);
        remove(path);
        
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
 * pumpd -- the function executed by the running daemon
 */
void pumpd(void)
{
        for (;;) {

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

        pid = readpid(PIDDIR);

        kill(pid, SIGTERM);
}


/**
 * pumpd_usage -- print the usage statement 
 */
void pumpd_help(void)
{
        printf("Help!\n");
}


int main(int argc, char *argv[])
{
        if (argc == 1)
                pumpd_help();

        else if (isarg(1, "start"))
                pumpd_start();

        else if (isarg(1, "stop"))
                pumpd_stop();

        else if (isarg(1, "help") || isarg(1, "?"))
                pumpd_help();

        return 0;
}




