#ifndef _DAEMON_LIBRARY_H
#define _DAEMON_LIBRARY_H


/* PID management */
int pidfile(const char *path, const char *mode);

/* Process grooming */ 
void daemonize(void);

/* FIFO operations */
void new_fifo(const char *path, int perms);
int open_fifo(const char *path, const char *mode);
void close_fifo(int fd);

/* FIFO I/O */
void fifo_write(int fd, void *buf, size_t len);
void fifo_read(int fd, void *buf, size_t len);

enum dpx_role { PUBLISH, SUBSCRIBE };
#define MIN_PIPESIZE (4095) // 4096 - 1
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
void dpx_read  (struct dpx_t *dpx);
void dpx_write (struct dpx_t *dpx);

#endif
