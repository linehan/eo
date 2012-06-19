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

char cwd[1024];

// permissions: drwxr-xr-x
#define PUMP_DIR "./.pump" 
#define PUMP_CONF "./.pump/config"
#define PUMP_LOGIC "./.pump/logic"
#define PUMP_DIR_MODE ((S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))

void pump_load(void)
{
        /* Get current working directory */
        if (getcwd(cwd, 1024) == NULL)
                bye("Could not stat working directory.");
}


bool is_pump_directory(const char *path)
{
        ASSERT(path != NULL);
        static char pumpdir[1024];
        struct stat buffer;

        sprintf(pumpdir, "%s/.pump", path);

        if (stat(pumpdir, &buffer) == -1) {
                if (errno == ENOENT)
                        return false;
                else
                        bye("Cannot stat pump directory");
        }
        return true;
}



void pump_init(void)
{
        FILE *conf;

        /* Create the .pump directory */
        if (mkdir(PUMP_DIR, PUMP_DIR_MODE) == -1)
                bye("Could not create pump directory.");

        /* Create config */
        if (conf = fopen(PUMP_CONF, "w+"), conf == NULL)
                bye("Could not create config");

        /* Print the default configuration in config */
        fprintf(conf, 
                        "# --------------------------\n"
                        "# default pump configuration\n"
                        "# --------------------------\n"
                        "\n"
                        "# Full path of directory to be pumped\n"
                        "basedir \"%s\"\n"
                        "\n", 
                cwd);

        /* Close config */
        if (fclose(conf) == EOF)
                bye("Could not close config");
}

void pump_logic(const char *statement)
{
        FILE *logic;

        if (!is_pump_directory(cwd))
                bye("Not a pump directory");

        /* Create logic */
        if (logic = fopen(PUMP_LOGIC, "w+"), logic == NULL)
                bye("Could not create logic");

        /* Print the default configuration in config */
        fprintf(logic, 
                        "# --------------------------\n"
                        "# pump logic\n"
                        "# --------------------------\n"
                        "\n"
                        "\"%s\"\n"
                        "\n", 
                statement);

        /* Close logic */
        if (fclose(logic) == EOF)
                bye("Could not close logic");
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

