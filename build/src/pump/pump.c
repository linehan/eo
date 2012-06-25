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
#include "../common/channel.h"
#include "../common/curses.h"
#include "../common/configfiles.h"


/******************************************************************************
 * PUMP
 ******************************************************************************/

/**
 * usage -- Print the usage message to stdout and exit
 * Returns nothing.
 */
void usage(void)
{
        printf("%s", USAGE_MESSAGE);
        exit(0);
}


/**
 * pump_init -- Initialize a pump in the current working directory
 * Returns nothing.
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
 * pump_logic -- Specify the logic that will drive the pump
 * @statement: the logic statement
 * Returns nothing.
 */
void pump_logic(const char *logic)
{
        if (!is_pump(ENV.cwd)) 
                bye("Not a pump directory");

        setconfig(ENV.config, "link", logic); 
}


/**
 * pump_say -- Echo a message to stdout via the pump daemon
 * @msg: Text to be printed
 * Returns nothing.
 */
/*void pump_say(char *msg)*/
/*{*/
        /*struct dpx_t dpx;*/

        /*dpx.role = SUBSCRIBE;*/
        /*dpx_open(&dpx, FIFO_PATH);*/

        /*dpx_load(&dpx, msg);*/
        /*dpx_write(&dpx);*/
        /*dpx_read(&dpx);*/

        /*printf("%s\n", dpx.buf);*/

        /*dpx_close(&dpx);*/
/*}*/


void pump_try(char *path)
{
        struct dpx_t dpx;
        char target[512];

        dpx.role = SUBSCRIBE;
        dpx_open(&dpx, CFG_PATH);

        dpx_load(&dpx, path);
        dpx_write(&dpx);
        dpx_read(&dpx);

        strcpy(target, dpx.buf);
        printf("target: %s\n", target);

        dpx_close(&dpx);
        dpx_flush(&dpx);
        dpx_open(&dpx, target);

        for (;;) {
                dpx_read(&dpx);
                printf("%s\n", dpx.buf);
        }
}


/******************************************************************************
 * MAIN 
 ******************************************************************************/
int main(int argc, char *argv[]) 
{
        load_env(&ENV);

        /*for (i=0; i<argc; i++) {*/
                /*printf("%s\n", argv[i]);*/
        /*}*/

        if (!ARG(1))
                usage();

        else if (isarg(1, "init"))
                pump_init();

        else if (isarg(1, "logic"))
                (ARG(2)) ? pump_logic(ARG(2)) : usage();

        else if (isarg(1, "var"))
                (ARG(2)) ? printf("%s\n", token(ARG(2), ENV.config)) : usage();

        /*else if (isarg(1, "say"))*/
                /*(ARG(2)) ? pump_say(ARG(2)) : usage();*/

        else if (isarg(1, "try"))
                (ARG(2)) ? pump_try(ARG(2)) : usage();

        return 0;
}

