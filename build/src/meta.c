#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "meta.h"
#include "common/io/file.h"
#include "common/textutils.h"
#include "common/error.h"

/* drwxr-xr-x */
#define PUMP_DIR_MODE ((S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))


/* Pathnames indexed by file tag */
#define pump_stem   "/.eo" 
#define config_stem "/.eo/config"
#define CONF_PATH   "./.eo/config"


/**
 * load_env -- fill out the current working directory struture 
 * @environ: will contain the current environment 
 */
void load_env(struct env_t *environ)
{
        if (getcwd(environ->cwd, PATHSIZE) == NULL)
                bye("Could not stat working directory.");

        snprintf(environ->pump,   PATHSIZE, "%s%s", environ->cwd, pump_stem);
        snprintf(environ->config, PATHSIZE, "%s%s", environ->cwd, config_stem);
}


/****************************************************************************** 
 * PUMP DIRECTORY MANAGEMENT 
 * 
 * The path argument accepted by all of these refers to a working
 * directory which is the root of the directory that contains the
 * .pump subfolder, NOT the actual path of a particular file.
 ******************************************************************************/


/**
 * make_pump 
 * `````````
 * Create a ./.pump directory to hold pump metadata
 *
 * @path : the root directory in which to create .pump
 * Return: nothing.
 * 
 * NOTE
 * This function sets the diagnostic condition of the is_pump() predicate. 
 * Whether a given directory is or is not a pump is judged entirely on the 
 * existence of a .pump subdirectory.
 */
void make_pump(const char *path)
{
        if (mkdir(concat(path, pump_stem), PUMP_DIR_MODE) == -1)
                bye("Could not create pump directory.");
}


/**
 * is_pump 
 * ```````
 * Test if 'path' contains a .pump subdirectory (is a pump)
 *
 * @path : the directory in which to look for a .pump subdirectory
 * Return: true if .pump subdirectory detected, else false
 */
bool is_pump(const char *path)
{
        struct stat buffer;

        if (stat(concat(path, pump_stem), &buffer) == -1) {
                if (errno == ENOENT)
                        return false;
                else
                        bye("Cannot stat pump directory");
        }
        return true;
}


/**
 * readconfig -- fill a pumpconfig structure with the config file at 'path'
 * @config: pumpconfig struct
 * @path  : path to the config file to be parsed
 */
void read_config(struct pumpconfig_t *config, const char *path)
{
        if (!exists(CONF_PATH)) {
                printf("This does not appear to be a pump directory.\n");
                exit(1);
        } else {
                get_token(config->name, "name", path);
                get_token(config->desc, "desc", path);
                get_token(config->base, "base", path);
                get_token(config->sha2, "sha2", path);
                get_token(config->link, "link", path);
                get_token(config->wait, "wait", path);
        }
}






/**
 * writeconfig -- write a pumpconfig structure to a config file at 'path'
 * @config: pumpconfig struct
 * @path  : path to the config file to be written
 */
void write_config(struct pumpconfig_t *config, const char *path)
{
        FILE *configfile;

        configfile = sopen(path, "w+");
        fprintf(configfile, 
                        "# --------------------------\n"
                        "# PUMP CONFIGURATION\n"
                        "# --------------------------\n\n"
                        "# Pump name\n"
                        "name \"%s\"\n\n"
                        "# Pump description\n"
                        "desc \"%s\"\n\n"
                        "# Directory to be pumped\n"
                        "base %s\n\n"
                        "# Identifies pump to the pump daemon\n"
                        "sha2 %s\n\n"
                        "# Link to external logic\n"
                        "link \"%s\"\n\n"
                        "# Delay between pumps (in seconds)\n"
                        "wait %s\n\n",
                config->name, 
                config->desc, 
                config->base, 
                config->sha2,
                config->link,
                config->wait);

        sclose(configfile);
}


/**
 * pumpconfig -- manually supply the members of the pumpconfig struct
 * @config: pumpconfig struct
 * @n: name 
 * @d: desc 
 * @b: base 
 * @s: sha2 
 * @l: link
 * @w: wait 
 */
void pumpconfig(struct pumpconfig_t *config, 
                char *n, char *d, char *b, char *s, char *l, char *w)
{
        char *name, *desc, *base, *sha2, *link, *wait;

        name = n ? n : "";
        desc = d ? d : "";
        base = b ? b : "";
        sha2 = s ? s : "";
        link = l ? l : "";
        wait = w ? w : "";

        snprintf(config->name, LINESIZE, "%s", name);
        snprintf(config->desc, LINESIZE, "%s", desc);
        snprintf(config->base, LINESIZE, "%s", base);
        snprintf(config->sha2, LINESIZE, "%s", sha2);
        snprintf(config->link, LINESIZE, "%s", link);
        snprintf(config->wait, LINESIZE, "%s", wait);
}


/**
 * setconfig -- set the value of a config file token 
 * @path : config file path
 * @token: token whose value will be changed 
 * @value: new value of token
 */
void set_config(const char *path, const char *token, const char *value)
{
        struct pumpconfig_t config;

        read_config(&config, path);

        if (STRCMP(token, "name"))
                snprintf(config.name, LINESIZE, "%s", value);
        else if (STRCMP(token, "desc"))
                snprintf(config.desc, LINESIZE, "%s", value);
        else if (STRCMP(token, "base"))
                snprintf(config.base, LINESIZE, "%s", value);
        else if (STRCMP(token, "sha2"))
                snprintf(config.sha2, LINESIZE, "%s", value);
        else if (STRCMP(token, "link"))
                snprintf(config.link, LINESIZE, "%s", value);
        else if (STRCMP(token, "wait"))
                snprintf(config.wait, LINESIZE, "%s", value);
        else
                bye("Invalid token passed to setconfig()");

        write_config(&config, path);
}


void print_config(void)
{
        struct pumpconfig_t config;

        read_config(&config, CONF_PATH); 

        printf("name: %s\n", config.name);
        printf("desc: %s\n", config.desc);
        printf("base: %s\n", config.base);
        printf("sha2: %s\n", config.sha2);
        printf("link: %s\n", config.link);
        printf("wait: %s\n\n", config.wait);
}


