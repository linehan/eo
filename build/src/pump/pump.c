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


#define FIFO_SUB "fifo.sub"
#define FIFO_PUB "fifo.pub"
#define CFG_STEM ".pump"


#define FIFO_PATH (CONCAT((gethome()), ("/"CFG_STEM"/fifo")))


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

        dpx.role = SUBSCRIBE;
        dpx_open(&dpx, FIFO_PATH);

        dpx_load(&dpx, message);
        dpx_write(&dpx);
        dpx_read(&dpx);

        printf("%s\n", dpx.buf);

        dpx_close(&dpx);
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

