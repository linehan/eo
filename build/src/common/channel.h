#ifndef _IPC_CHANNELS_H 
#define _IPC_CHANNELS_H 


#define MIN_PIPESIZE (4095) // 4096 - 1

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
};


void dpx_creat (const char *dpxname, int perms);
void dpx_remove(const char *dpxname);

void dpx_open  (struct dpx_t *dpx, const char *path);
void dpx_close (struct dpx_t *dpx);
void dpx_load  (struct dpx_t *dpx, char *msg);
void dpx_flush (struct dpx_t *dpx);
void dpx_read  (struct dpx_t *dpx);
void dpx_write (struct dpx_t *dpx);


#endif
