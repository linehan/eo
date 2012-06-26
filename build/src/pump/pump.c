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
#include "../common/textutils.h"


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


void pump_list(char *path)
{
        struct dpx_t dpx;
        char abspath[255];

        if (is_relpath(path))
                rel2abs(path, abspath);
        else
                strcpy(abspath, path);

        /* Subscribe to the pump daemon's control channel */
        dpx_open(&dpx, CHANNEL("control"), CH_SUB);

        dpx_send(&dpx, abspath); // Send the path we want to be listed
        dpx_read(&dpx);          // Wait for channel to connect to... 

        /* Print the channel (diagnostic) */
        printf("name: %s\nchan: %s\n\n", dpx.buf, CHANNEL(dpx.buf));

        /* Close the control channel and open the new channel */
        dpx_close(&dpx);
        dpx_open(&dpx, CHANNEL(dpx.buf), CH_SUB);
        dpx_send(&dpx, "ack"); // Tell the pump that we're ready to receive

        /* Receive file listing until "done" message from pump */
        for (;;) {
                dpx_read(&dpx);
                if (STRCMP(dpx.buf, "done")) {
                        dpx_send(&dpx, "ack"); // Tell pump we received "done"
                        break;
                }
                printf("%s\n", dpx.buf);
        }
        /*cmp_dir(abspath, F_REG);*/
        dpx_close(&dpx);
}


/******************************************************************************
 * MAIN 
 ******************************************************************************/
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

        else if (isarg(1, ":-"))
                (ARG(2)) ? pump_list(ARG(2)) : usage();

        return 0;
}

