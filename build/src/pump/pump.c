#define __MERSENNE__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "pump.h"
#include "meta.h"
#include "../common/file.h"
#include "../common/error.h"
#include "../common/util.h"
#include "../common/daemon.h"


#define HOME "/home/linehan/.pump"
#define FIFO_SUB "fifo.sub"
#define FIFO_PUB "fifo.pub"
#define PUB_PATH HOME"/"FIFO_PUB
#define SUB_PATH HOME"/"FIFO_SUB


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

        pumpconfig(&config, 
                        "Pumping Flash", 
                        "The prime mover", 
                        ENV.cwd, 
                        hex, 
                        "/foo/bar/bazscript.qux",
                        "10"
                  );

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


void pump_say(char *message)
{
        struct dpx_t dpx;
        char buffer[1024];

        dpx.role = SUBSCRIBE;
        open_dpx(&dpx, PUB_PATH, SUB_PATH);

        fifo_write(dpx.fd_pub, (void *)message, (strlen(message)));
        fifo_read(dpx.fd_sub, buffer, 1024);

        printf("%s\n", buffer);

        close_dpx(&dpx);
}


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
                (ARG(2)) ? pump_say(ARG(2)) : usage();

        return 0;
}

