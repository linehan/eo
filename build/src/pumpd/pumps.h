#ifndef _PUMPS_H
#define _PUMPS_H

struct pump_t *__pump_fw;

#define P_FORK 1
#define P_KEEP 0

struct pump_t *new_pump(char *target, char *channel, int mode);
void          open_pump(struct pump_t *p);
void          kill_pump(struct pump_t *p);
void         pump_files(struct pump_t *p);

#endif
