#define __MERSENNE__
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>

#include "pump.h"
#include "meta.h"
#include "../common/file.h"
#include "../common/error.h"
#include "../common/util.h"
#include "../common/daemon.h"

#define FIFO_WRITE "/home/linehan/src/mine/pump/pumpd/pumpd.read"
#define FIFO_READ  "/home/linehan/src/mine/pump/pumpd/pumpd.write"

/**
 * usage -- print the usage message to stdout and exit
 */
void usage(void)
{
        printf("%s", USAGE_MESSAGE);
        exit(0);
}


/**
 * pump_init -- initialize a pump in the current working directory
 */
void pump_init(void)
{
        struct pumpconfig_t config;
        unsigned long salt;
        char hex[65];

        if (is_pump(ENV.cwd)) 
                bye("pump exists");

        make_pump(ENV.cwd);

        salt = mt_random();
        sha256gen(hex, &salt);

        pumpconfig(&config, "Pumpalicious", "The prime pumper", ENV.cwd, hex, NULL, NULL);

        writeconfig(&config, ENV.config);
}


/**
 * pump_logic -- specify the logic that will drive the pump
 * @statement: the logic statement
 */
void pump_logic(const char *logic)
{
        if (!is_pump(ENV.cwd)) 
                bye("Not a pump directory");

        setconfig(ENV.config, "link", logic); 
}


void pump_write(char *message)
{
        int read, write;
        char *msg = "This is a test of the System V IPC\n";
        char buffer[1024];

        write = open_fifo(FIFO_WRITE, 'w');
        read = open_fifo(FIFO_READ, 'r');

        fifo_write(write, (void *)message, (strlen(message)));
        fifo_read(read, buffer, 1024);

        printf("%s\n", buffer);

        close_fifo(read);
        close_fifo(write);
}

/**
 * pump_start -- gather the metadata and register the pump with pumpd
 */
/*void pump_start(void)*/
/*{*/
        /*char *buf[1024];*/
        /*struct pump_meta *meta;*/

        /*meta = getmeta();*/

        /*system("pumpd -n ");*/


/**
 * main -- it's main
 */
int main(int argc, char *argv[]) 
{
        load_env(&ENV);

        if (!ARG(1))
                usage();

        else if (isarg(1, "init"))
                pump_init();

        else if (isarg(1, "logic"))
                (ARG(2)) ? pump_logic(ARG(2)) : usage();

        else if (isarg(1, "var"))
                (ARG(2)) ? printf("%s\n", token(ARG(2), ENV.config)) : usage();

        else if (isarg(1, "say"))
                (ARG(2)) ? pump_write(ARG(2)) : usage();

        return 0;
}

