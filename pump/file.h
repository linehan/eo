#ifndef _PUMP_FILES_H
#define _PUMP_FILES_H

#include "../common/util.h"

#define PATHSIZE 255
#define LINESIZE 1024

struct env_t {
        char cwd[PATHSIZE];
        char config[PATHSIZE];
        char logic[PATHSIZE];
        char pump[PATHSIZE];
};

struct env_t ENV;


struct pumpconfig_t {
        char name[LINESIZE];
        char desc[LINESIZE];
        char base[LINESIZE];
        char sha2[LINESIZE];
        char link[LINESIZE];
        char wait[LINESIZE];
};

void load_env(struct env_t *environ);

bool exists(const char *path);

FILE *pumpfile_open(const char *path, const char *mode);
void pumpfile_close(FILE *file);

void make_pump(const char *path);
bool is_pump(const char *path);
bool has_logic(const char *path);

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

void get_token(char *dest, const char *token, const char *path);
char *token(const char *token, const char *path);

void readconfig(struct pumpconfig_t *config, const char *path);
void writeconfig(struct pumpconfig_t *config, const char *path);
void setconfig(const char *path, const char *token, const char *value);

void pumpconfig(struct pumpconfig_t *config, 
                char *n, char *d, char *b, char *s, char *l, char *w);

#endif
