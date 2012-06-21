int pidfile(const char *path, const char *mode);
void daemonize(void);

void new_fifo(const char *path, int perms);

int open_fifo(const char *path, int mode);

FILE *stream_fifo(const char *path, const char *mode);
void close_fifo(int fd);

void fifo_write(int fd, void *buf, size_t len);
void fifo_read(int fd, void *buf, size_t len);
