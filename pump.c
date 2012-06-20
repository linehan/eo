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
void print_info_usage(void)
{
        printf("pump info usage statement\n");
        exit(0);
}



/**
 * do_pump -- apply the logic across each file of the directory
 */
void do_pump(void)
{
        DIR *dir;
        char *filename;
        char execute[512];
        char *logic=".pump/logic";

        dir = opendir("./");

        for (filename  = getfile(dir, F_REG);
             filename != NULL;
             filename  = getfile(NULL, F_REG))
        {
                sprintf(execute, "./%s %s", logic, filename);
                system(execute);
        }

        closedir(dir);
}

/**
 * sha256gen -- return a hex string of the sha256sum
 * @hex : will be filled with the sha256sum. Must be at least 64 bytes
 * @hash: the data used to generate the sha256sum
 */
void sha256gen(unsigned char *hex, void *hash)
{
        #define SHA32 32
        sph_sha256_context context;
        unsigned char output[SHA32];

        sph_sha256_init(&context);
        sph_sha256(&context, hash, sizeof(hash));
        sph_sha256_close(&context, output);

        strtohex(hex, output, SHA32);
}


/**
 * pump_init -- initialize a pump in the current working directory
 */
void pump_init(void)
{
        FILE *config;
        unsigned char hex[65];

        if (is_pump(CWD)) 
                bye("pump exists");

        make_pump();

        sha256gen(hex, CWD);

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

        if (!is_pump(CWD)) 
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

        if (!ARG(1))
                (is_pump(CWD)) ? do_pump() : print_usage();

        else if (isarg(1, "init"))
                pump_init();

        else if (isarg(1, "info"))
                (ARG(2)) ? pump_info(ARG(2)) : print_info_usage();

        else if (isarg(1, "logic"))
                (ARG(2)) ? pump_logic(ARG(2)) : print_logic_usage();

        else if (isarg(1, "var"))
                (ARG(2)) ? printf("%s", getvar(ARG(2))) : print_logic_usage();

        return 0;
}

