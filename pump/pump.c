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

#include "file.h"
#include "../common/error.h"
#include "../common/util.h"


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
void print_info_usage(void)
{
        printf("pump info usage statement\n");
        exit(0);
}


/**
 * pump_init -- initialize a pump in the current working directory
 */
void pump_init(void)
{
        struct pumpconfig_t config;
        char hex[65];
        unsigned long salt;

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
                print_usage();

        else if (isarg(1, "init"))
                pump_init();

        else if (isarg(1, "info"))
                (ARG(2)) ? pump_info(ARG(2)) : print_info_usage();

        else if (isarg(1, "logic"))
                (ARG(2)) ? pump_logic(ARG(2)) : print_logic_usage();

        else if (isarg(1, "var"))
                (ARG(2)) ? printf("%s\n", token(ARG(2), ENV.config)) : print_logic_usage();

        return 0;
}

