#ifndef _PUMP_FILES_H
#define _PUMP_FILES_H

enum files { PUMP, CONFIG, LOGIC };
static const char *filename[]={ "pump directory", "config", "logic" };

static char CWD[1024];

void make_pump(void);
void pump_load(void);
FILE *pump_open(enum files tag, const char *mode);
void pump_close(FILE *file);
bool is_pump_directory(void);

#endif
