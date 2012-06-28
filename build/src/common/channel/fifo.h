#ifndef _IPC_FIFO_H
#define _IPC_FIFO_H

#define PERMS (0666)
#define DIR_PERMS ((S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH))

void fifo_creat (const char *path, int perm);
void fifo_remove(const char *path);
int  fifo_open  (const char *path, mode_t mode);
void fifo_close (int fd);
void fifo_read  (int fd, void *buf, size_t len);
void fifo_write (int fd, void *buf, size_t len);

#endif
