#ifndef _PUMP_METADATA_H
#define _PUMP_METADATA_H

#define PATHSIZE 255
#define LINESIZE 1024

struct env_t {
        char cwd[PATHSIZE];
        char config[PATHSIZE];
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
        char nice[LINESIZE];
};

void load_env(struct env_t *environ);
void make_pump(const char *path);
bool is_pump(const char *path);
bool has_logic(const char *path);


void readconfig(struct pumpconfig_t *config, const char *path);
void writeconfig(struct pumpconfig_t *config, const char *path);
void setconfig(const char *path, const char *token, const char *value);

void pumpconfig(struct pumpconfig_t *config, 
                char *n, char *d, char *b, char *s, char *l, char *w);

#endif
