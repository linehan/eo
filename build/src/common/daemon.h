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
        int fd_nub; // sticky fd for loop driver
        char buf[MIN_PIPESIZE];
};

void  open_dpx(struct dpx_t *dpx, const char *pub_path, const char *sub_path);
void close_dpx(struct dpx_t *dpx);

void load_dpx(struct dpx_t *dpx, char *msg);

void  read_dpx(struct dpx_t *dpx);
void write_dpx(struct dpx_t *dpx);

#endif
