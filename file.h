#ifndef _PUMP_FILES_H
#define _PUMP_FILES_H

enum files { PUMPDIR, CONFIG, LOGIC };

/* Current working directory */
char CWD[1024];

/* Config file formatting */
#define BAR "# --------------------------\n"
#define CONFIG_BANNER   BAR "# default pump configuration\n" BAR "\n"
#define LOGIC_BANNER    BAR "# pump logic\n" BAR "\n"
#define CONFIG_BASEDIR  "# Directory to be pumped\n basedir \"%s\"\n\n"
#define LOGIC_STATEMENT "# Logic statement\n \"%s\"\n\n"


FILE *pump_open(enum files tag, const char *mode);
void pump_close(FILE *file);

void make_pump(void);
bool is_pump(void);

void load_cwd(void);



#endif
