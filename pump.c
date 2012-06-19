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

void pump_init(void)
{
        FILE *conf;

        if (is_pump_directory())
                bye("pump exists");

        make_pump();
        conf = pump_open(CONFIG, "w+");

        /* Print the default configuration in config */
        fprintf(conf, 
                        "# --------------------------\n"
                        "# default pump configuration\n"
                        "# --------------------------\n"
                        "\n"
                        "# Full path of directory to be pumped\n"
                        "basedir \"%s\"\n"
                        "\n", 
                CWD);

        pump_close(conf);
}

void pump_logic(const char *statement)
{
        FILE *logic;

        if (!is_pump_directory())
                bye("Not a pump directory");

        logic = pump_open(LOGIC, "w+");

        /* Print the default configuration in config */
        fprintf(logic, 
                        "# --------------------------\n"
                        "# pump logic\n"
                        "# --------------------------\n"
                        "\n"
                        "\"%s\"\n"
                        "\n", 
                statement);

        pump_close(logic);
}




int main(int argc, char *argv[]) 
{
        pump_load();

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

