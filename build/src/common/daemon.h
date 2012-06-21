
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

