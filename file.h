#ifndef _PUMP_FILES_H
#define _PUMP_FILES_H

enum files { PUMPDIR, CONFIG, LOGIC };
char CWD[1024]; /* Current working directory */


#define BAR "# --------------------------\n"
#define CONFIG_BANNER   BAR "# default pump configuration\n" BAR "\n"
#define LOGIC_BANNER    BAR "# pump logic\n" BAR "\n"
#define LOGIC_STATEMENT "# Logic statement\n\"%s\"\n\n"
#define CONFIG_BASEDIR  "# Directory to be pumped\nbasedir \"%s\"\n\n"
#define CONFIG_IDENT    "# Identifies pump to the pump daemon (sha256)\nident \"%s\"\n\n"

FILE *pump_open(enum files tag, const char *mode);
void pump_close(FILE *file);

void make_pump(void);
bool is_pump(void);

void load_cwd(void);


void list_dir(DIR *dir, int options);
#define LPRM (1 << 1)
#define LUSR (1 << 2)
#define LGRP (1 << 3)
#define LSIZ (1 << 4)
#define LDAT (1 << 5)
#define LNAM (1 << 6)
#define LHID (1 << 7)
#define LDIR (1 << 8)
#define LREG (1 << 9)

int filecount(DIR *dir, int options);
char *getfile(DIR *dir, int options);

#endif
