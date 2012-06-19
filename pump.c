#define USE_ERRNO_H

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

#include "file.h"
#include "error.h"
#include "util.h"
#include "sha256/sph_sha2.h"

void print_usage(void)
{
        printf("pump usage statement\n");
        exit(0);
}
void print_logic_usage(void)
{
        printf("pump logic usage statement\n");
        exit(0);
}


/**
 * pump_init -- initialize a pump in the current working directory
 */
void pump_init(void)
{
        FILE *config;
        unsigned char buf[32];
        unsigned char hex[65];
        sph_sha256_context context;

        sph_sha256_init(&context);
        sph_sha256(&context, CWD, strlen(CWD));
        sph_sha256_close(&context, buf);

        strtohex(hex, buf, 32);

        if (is_pump()) 
                bye("pump exists");

        make_pump();

        config = pump_open(CONFIG, "w+");
        fprintf(config, CONFIG_BANNER CONFIG_BASEDIR CONFIG_IDENT, CWD, hex);

        pump_close(config);
}


/**
 * pump_logic -- specify the logic that will drive the pump
 * @statement: the logic statement
 */
void pump_logic(const char *statement)
{
        FILE *logic;

        if (!is_pump()) 
                bye("Not a pump directory");

        logic = pump_open(LOGIC, "w+");
        fprintf(logic, LOGIC_BANNER LOGIC_STATEMENT, statement);

        pump_close(logic);
}


/**
 * main -- it's main
 */
int main(int argc, char *argv[]) 
{
        load_cwd();

        if (!argv[1])
                print_usage();
        if (STRCMP(argv[1], "init"))
                pump_init();
        else if (STRCMP(argv[1], "logic")) {
                if (!argv[2])
                        print_logic_usage();
                else
                        pump_logic(argv[2]);
        }
        return 0;
}

