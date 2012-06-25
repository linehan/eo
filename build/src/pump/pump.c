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


/******************************************************************************
 * FILES AND DIRECTORIES 
 ******************************************************************************/

#define FIFO_SUB "fifo.sub"
#define FIFO_PUB "fifo.pub"

/*
 * Home directory
 *
 * The configuration folder will be placed in the home directory
 * of the user who the process is associated with. See gethome()
 * in ../common/file.c.
 */
#define HOME_DIR  (gethome())


/*
 * Configuration files on disk
 *
 * The pump daemon maintains a number of files, all of which are
 * stored in a hidden directory, CFG_STEM, which resides in the
 * directory identified by CFG_PATH. 
 *
 * CFG_NAME     Name of the hidden directory
 * CFG_PATH     Path of the hidden directory
 *
 */
#define CFG_STEM  ".pump"
#define CFG_PATH  (CONCAT(HOME_DIR, "/"CFG_STEM))


/*
 * PID file
 *
 * In order to signal and stat the daemon, a client needs to know 
 * the pid (process id) of the daemon process. When the daemon is
 * started, it writes this number to the pid file.
 */
#define PID_NAME  "pumpd.pid"
#define PID_PATH  (CONCAT(CFG_PATH, "/"PID_NAME))


/*
 * FIFO files
 *
 * Communication between the daemon and clients is performed via
 * FIFO files (named pipes). A number of these files may need to
 * be maintained, depending on the number of clients and the amount
 * of message multiplexing. 
 *
 * FIFOs which carry messages *to* the server are marked with the 
 * extension ".sub", while those carrying messages *from* the server 
 * are marked with the extension ".pub". 
 *
 * These extensions are automatically appended to the path supplied 
 * to the dpx_creat() function (see daemon.c).
 */
#define FIFO_NAME "fifo"
#define FIFO_PATH (CONCAT(CFG_PATH, "/"FIFO_NAME))



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
void pump_say(char *msg)
{
        struct dpx_t dpx;

        dpx.role = SUBSCRIBE;
        dpx_open(&dpx, FIFO_PATH);

        dpx_load(&dpx, msg);
        dpx_write(&dpx);
        dpx_read(&dpx);

        printf("%s\n", dpx.buf);

        dpx_close(&dpx);
}


/******************************************************************************
 * MAIN 
 ******************************************************************************/
int main(int argc, char *argv[]) 
{
        int i;
        load_env(&ENV);

        for (i=0; i<argc; i++) {
                printf("%s\n", argv[i]);
        }

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

