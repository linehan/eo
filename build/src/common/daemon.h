#ifndef _DAEMON_LIBRARY_H
#define _DAEMON_LIBRARY_H


/* PID management */
int pidfile(const char *path, const char *mode);

/* Process grooming */ 
int daemonize(void);


#endif
