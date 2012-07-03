#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ipc.h>
#include <stdarg.h>

#include "../io/file.h"
#include "../error.h"
#include "../textutils.h"
#include "fifo.h"


/******************************************************************************
 * FIFOs (NAMED PIPES)
 *
 * Primitive calls intended to be used by the duplex API only.
 *
 * NOTES 
 *
 * A named pipe is a native filetype in most Unix filesystems.
 * It behaves similarly to the shell's pipe functionality, although
 * the implementation is distinct.
 *
 * Unlike regular files, a named pipe must be opened twice, once 
 * for reading and once for writing. It enforces a first-in, first-out
 * data transfer between the 'write' file descriptor and the 'read' file
 * descriptor. 
 *
 * The file itself does not know and does not care about the identity of 
 * the reader or writer. This means the reader and writer need not be the 
 * same process. Two unrelated processes can communicate by sharing data 
 * through a named pipe, with synchronization being ensured by the FIFO 
 * nature of the information exchange.
 * 
 * CAVEAT 
 *
 * (Amended from Stevens[90]) 
 *
 * A FIFO follows these rules for reading and writing:
 *
 * 0. A read requesting less data than is in the pipe or FIFO returns
 *    only the requested amount of data. The remainder is left in the
 *    FIFO and can be requested by subsequent reads.
 *
 * 1. A read requesting more data than is in the pipe or FIFO returns
 *    only the requested amount of data. The reader must be prepared
 *    to handle a return value that is less than the requested amount.
 *
 * 2. If the pipe or FIFO is empty, and if no processes have it open 
 *    for writing, a read returns 0, signifying EOF (end of file). 
 *    If the reader has specified O_NDELAY, it cannot tell if a return
 *    value of 0 means there is no data currently availible or if there
 *    are no writers left.
 *
 * 3. If a process writes less than the capacity of a pipe or FIFO
 *    (which is at least 4096 bytes) the write is guaranteed to be
 *    atomic. There is no guarantee otherwise.
 *
 * 4. If a process writes to a pipe or FIFO, but there are no processes 
 *    in existence that have it open for reading, the SIGPIPE signal is 
 *    generated, and the write returns 0 with errno set to EPIPE. If the 
 *    process has not handled the signal in some other way, the default 
 *    action is to terminate the process.
 *
 ******************************************************************************/

/**
 * fifo_creat 
 * ``````````
 * Create a new named pipe file
 *
 * @path : path of the new file 
 * @perm : permissions word
 * Return: nothing
 */
void fifo_creat(const char *path, int perm)
{
        if ((mknod(path, S_IFIFO | perm, 0)) == -1) {
                bye("Fee! Fie! Foe! Fum! The FIFO failed to open!");
        }
}


/**
 * fifo_remove
 * ```````````
 * Remove a named pipe from the filesystem 
 *
 * @path : path of the new file 
 * Return: nothing
 */
void fifo_remove(const char *path)
{
        if ((unlink(path)) == -1) {
                bye("Could not remove named pipe at %s", path);
        }
}


/**
 * fifo_open 
 * `````````
 * Open a named pipe with the specified mode. 
 *
 * @path : path to the already-created FIFO
 * @mode : any OR'ed combination of O_RDONLY, O_WRONLY, and O_NDELAY 
 * Return: file descriptor for the FIFO 
 */
int fifo_open(const char *path, mode_t mode)
{
        int fd;

        if ((fd = open(path, mode)) < 0)
                bye("daemon: Could not open fifo");

        return fd;
}


/**
 * fifo_close
 * ``````````
 * Close an open named pipe. 
 *
 * @fd   : file descriptor of the file to be closed
 * Return: nothing 
 */
void fifo_close(int fd)
{
        if ((close(fd)) < 0)
                bye("daemon: Could not close fifo");
}
        

/**
 * fifo_read  
 * `````````
 * Read the contents of a named pipe into a buffer.
 *
 * @fd   : file descriptor
 * @buf  : buffer to be written to fifo
 * @len  : size of the buffer
 * Return: nothing
 */
void fifo_read(int fd, void *buf, size_t len)
{
        #define NULterminate
        size_t z;

        if ((z = read(fd, buf, len)) == -1)
                bye("daemon: Could not read from fifo");

        #if defined(NULterminate)
        ((char *)buf)[z] = '\0';
        #endif
}


/**
 * fifo_write 
 * ``````````
 * Write the contents of a buffer into a named pipe.
 *
 * @fd   : file descriptor
 * @buf  : buffer to be written to fifo
 * @len  : size of the buffer
 * Return: nothing
 */
void fifo_write(int fd, void *buf, size_t len)
{
        if ((write(fd, buf, len)) == -1)
                bye("daemon: Could not write to fifo");
}

