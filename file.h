#ifndef _PUMP_FILES_H
#define _PUMP_FILES_H

enum files { PUMPDIR, CONFIG, LOGIC };
char CWD[1024]; /* Current working directory */


#define BAR "# --------------------------\n"
#define CONFIG_BANNER   BAR "# default pump configuration\n" BAR "\n"
#define LOGIC_BANNER    BAR "# pump logic\n" BAR "\n"
#define LOGIC_STATEMENT "# Logic statement\n%s\n\n"
#define CONFIG_BASEDIR  "# Directory to be pumped\nbasedir %s\n\n"
#define CONFIG_IDENT    "# Identifies pump to the pump daemon (sha256)\nident %s\n\n"

FILE *pump_open(enum files tag, const char *mode);
void pump_close(FILE *file);

void make_pump(void);
bool is_pump(const char *path);

void load_cwd(void);


void list_dir(DIR *dir, int options);
/* List filters */
#define LPRM (1 << 1)   // Print permissions
#define LUSR (1 << 2)   // Print user (owner)
#define LGRP (1 << 3)   // Print group (owner)
#define LSIZ (1 << 4)   // Print size
#define LDAT (1 << 5)   // Print date
#define LNAM (1 << 6)   // Print filename
/* Operand filters */
#define F_HID (1 << 7)   // Filter for hidden files
#define F_DIR (1 << 8)   // Filter for directories
#define F_REG (1 << 9)   // Filter for regular files
#define F_SYM (1 << 10)  // Filter for symlinks
#define F_PIP (1 << 11)  // Filter for named pipes
#define F_SOC (1 << 12)  // Filter for sockets
#define F_DEV (1 << 13)  // Filter for block device files

int filecount(DIR *dir, int options);
char *getfile(DIR *dir, int options);
void pump_info(const char *path);
char *getvar(const char *name);

#endif
