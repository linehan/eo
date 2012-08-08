#ifndef _IPC_CHANNELS_H 
#define _IPC_CHANNELS_H 

#include <stdbool.h>
#include <sys/types.h>

#define MIN_PIPESIZE (4095) // 4096 - 1
#define MAX_PATHSIZE (255)

enum dpx_role { 
        PUBLISH, 
        SUBSCRIBE 
};

struct dpx_t {
        enum dpx_role role;
        int fd_pub; // publish on this fd
        int fd_sub; // consume on this fd 
        int fd_nub; // sticky fd for loop driver (keepalive)
        char buf[MIN_PIPESIZE];
        char *path_pub;
        char *path_sub;
        char *path;
        pid_t remote_pid;
};

#define CH_NEW 0x001
#define CH_PUB 0x002
#define CH_SUB 0x000
#define CH_ROLE(mode) (((mode) & CH_PUB) == CH_PUB) ? PUBLISH : SUBSCRIBE
#define NEW_CH(mode)  (((mode) & CH_NEW) == CH_NEW) ? true : false


void dpx_creat (const char *path);
void dpx_remove(const char *path);

void dpx_open  (struct dpx_t *dpx, const char *path, int mode);

void dpx_close (struct dpx_t *dpx);

void dpx_read  (struct dpx_t *dpx);
void dpx_write (struct dpx_t *dpx);

void dpx_send  (struct dpx_t *dpx, const char *msg);
void dpx_sendf (struct dpx_t *dpx, const char *fmt, ...);
void dpx_ping  (struct dpx_t *dpx, const char *msg);
void dpx_pingf (struct dpx_t *dpx, const char *fmt, ...);

void dpx_link  (struct dpx_t *dpx);
void dpx_load  (struct dpx_t *dpx, const char *msg);
void dpx_flush (struct dpx_t *dpx);
void dpx_kill  (struct dpx_t *dpx, int signo);


static inline void dpx_olink(struct dpx_t *dpx, const char *path, int mode)
{ 
        dpx_open(dpx, path, mode); 
        dpx_link(dpx); 
}


#endif
