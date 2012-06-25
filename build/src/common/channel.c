#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ipc.h>

#include "file.h"
#include "error.h"
#include "textutils.h"
#include "ipc.h"


#define PERMS (0666)


/******************************************************************************
 * FIFOs (NAMED PIPES)
 * Private primitive calls intended to be used by the duplex API only.
 *
 * Primer
 * ------
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
 * Operation
 * ---------
 *
 * A FIFO follows these rules for reading and writing:
 * (Amended from Stevens[90]) 
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
 * new_fifo -- Create a new named pipe file
 * @path: path of the new file 
 * @perm: permissions
 */
void new_fifo(const char *path, int perm)
{
        if ((mknod(path, S_IFIFO | perm, 0)) == -1) {
                bye("Fee! Fie! Foe! Fum! The FIFO failed to open!");
        }
}


/**
 * open_fifo -- open a FIFO with the specified mode 
 * @path: path to the already-created FIFO
 * @mode: any combination of 'r' 'w' 'k' and 'n' in a string
 * r=read only, w=write only, n=no delay (non-blocking), k=keep alive
 * Returns file descriptor for the FIFO 
 */
int open_fifo(const char *path, const char *mode)
{
        mode_t MODE;
        size_t len;
        int fd;

        len = strlen(mode);

        if ((memchar((void *)mode, 'r', len))) MODE  = O_RDONLY;
        if ((memchar((void *)mode, 'w', len))) MODE  = O_WRONLY;
        if ((memchar((void *)mode, 'n', len))) MODE |= O_NDELAY;

        if ((fd = open(path, MODE)) < 0)
                bye("daemon: Could not open fifo");

        /* keepalive -- see note (Stevens) above */
        if ((memchar((void *)mode, 'k', len))) open_fifo(path, "w");

        return fd;
}


/**
 * close_fifo -- close a FIFO file descriptor
 * @fd: file descriptor of the file to be closed
 * Returns nothing 
 */
void close_fifo(int fd)
{
        if ((close(fd)) < 0)
                bye("daemon: Could not close fifo");
}
        

/**
 * fifo_read -- read the contents of a fifo into a buffer
 * @fd : file descriptor
 * @buf: buffer to be written to fifo
 * @len: size of the buffer
 * Returns nothing
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
 * fifo_write -- write the contents of a buffer into a fifo
 * @fd : file descriptor
 * @buf: buffer to be written to fifo
 * @len: size of the buffer
 * Returns nothing
 */
void fifo_write(int fd, void *buf, size_t len)
{
        if ((write(fd, buf, len)) == -1)
                bye("daemon: Could not write to fifo");
}



/******************************************************************************
 * DUPLEX CHANNELS 
 * 
 * Primer
 * ------
 *
 * A channel is an IPC abstraction enforced (in this implementation) 
 * by a datatype struct dpx_t. This structure encapsulates three file 
 * descriptors, which associate to 2 FIFO files (named pipes). The 
 * extra descriptor is used to ensure that the channel remains open
 * after one end has hung up. 
 * 
 * A channel is a duplex system, in contrast to the POSIX FIFO which
 * is strictly serial. More precisely, it pretends to be a duplex
 * system, by opening two FIFOs, one for reading and one for writing.
 *
 * Because communication through a FIFO can only proceed
 * in one direction, from writer to reader, if we want two processes to
 * establish a two-way communication channel we must open two FIFOs at
 * once:
 *
 *                      FIFO 1
 *                =================
 *                write -----> read
 *                =================                                    
 *     PROCESS A                     PROCESS B 
 *                =================
 *                read <----- write
 *                =================
 *                      FIFO 2
 *
 *
 * Before a channel is opened, the caller must specify whether their
 * process will be a PUBLISHER or a SUBSCRIBER. This setting affects
 * which of the file descriptors is written on, and whether the channel
 * needs to be kept alive, and also allows the blocking logic to be
 * enforced below the API, where the caller can't screw it up!
 *
 * Avoiding deadlocks
 * ------------------
 *
 * Stevens[90] elaborates on some of the crap that gets taken care
 * of here:
 *
 *    "The order of the open calls is important, and avoids a deadlock
 *     condition. When the server opens FIFO1 for writing, it waits until
 *     the client opens it for reading. If the first call to open in the
 *     client were for FIFO2 instead of FIFO1, then the client would wait
 *     for the parent to open FIFO2 for writing. Each process would then
 *     be waiting for the other, and neither would proceed."
 *
 * There are two solutions here. You could specify O_NDELAY mode, 
 * and eliminate blocking altogether, or code the calls in the 
 * appropriate way. We're going with the latter option, although
 * no-delay is supported (see dpx_open()).
 *
 * "Keep-alive"
 * ------------
 *
 * As for the matter of keep-alive, again we consult Stevens[90]:
 *
 *    "The daemon opens the FIFO for read-only and its typical state 
 *     is waiting in a 'read' system call for a client request. 
 *
 *     Client processes are started and they open the FIFO for writing, 
 *     write their request, and exit. What happens is the 'read' 
 *     returns zero (end of file) to the daemon every time a client 
 *     process terminates, if no other clients have the FIFO open for 
 *     writing. 
 *
 *     The daemon has to then 'open' the FIFO again (for read-only) 
 *     and it waits here until a client process 'open's the 
 *     FIFO for writing. 
 *
 *     To avoid this, a useful technique is for the daemon to open the 
 *     FIFO two times -- once for reading and once for writing. The file 
 *     descriptor returned for reading is used to 'read' the client 
 *     requests, and the file descriptor for writing is never used. 
 *
 *     By having the FIFO always open for writing (as long as the daemon 
 *     process exists) the reads do not return an EOF, but wait for the 
 *     next client request."
 * 
 ******************************************************************************/

/**
 * dpx_creat -- create named FIFOs for a duplex channel
 * @dpxname: path of the FIFO (named pipe) to publish on 
 * @perms  : file permissions 
 * Returns nothing.
 */
void dpx_creat(const char *dpxname, int perms)
{
        new_fifo(CONCAT(dpxname, ".sub"), perms);
        new_fifo(CONCAT(dpxname, ".pub"), perms);
}


/**
 * dpx_remove -- destroy named FIFOs for a duplex channel
 * @pub_path
 */
void dpx_remove(const char *dpxname)
{
        sunlink(CONCAT(dpxname, ".sub"));
        sunlink(CONCAT(dpxname, ".pub"));
}
 

/**
 * dpx_open -- initialize a new duplex channel 
 * @dpx : pointer to a duplex structure
 * @path: path where FIFOs will be created (path.pub/path.sub)
 * Returns nothing.
 */
void dpx_open(struct dpx_t *dpx, const char *path)
{
        if (dpx->role == PUBLISH) {
                dpx->fd_sub = open_fifo(CONCAT(path, ".sub"), "r");
                dpx->fd_nub = open_fifo(CONCAT(path, ".sub"), "w");
                dpx->fd_pub = open_fifo(CONCAT(path, ".pub"), "w");
        } else if (dpx->role == SUBSCRIBE) {
                dpx->fd_pub = open_fifo(CONCAT(path, ".sub"), "w");
                dpx->fd_sub = open_fifo(CONCAT(path, ".pub"), "r");
        } else {
                bye("open_dpx: Invalid duplex role");
        }
}


/**
 * dpx_close -- close an open duplex channel
 * @dpx: pointer to a duplex structure
 * Returns nothing.
 */
void dpx_close(struct dpx_t *dpx)
{
        if (dpx->role == PUBLISH) {
                close(dpx->fd_sub);
                close(dpx->fd_nub);
                close(dpx->fd_pub);
        } else if (dpx->role == SUBSCRIBE) {
                close(dpx->fd_pub);
                close(dpx->fd_sub);
        } else {
                bye("close_dpx: Invalid duplex role");
        }
}


/**
 * dpx_load -- load a message buffer into the duplex struct 
 * @dpx: pointer to a duplex structure
 * @msg: message to be loaded in the duplex structure
 * Returns nothing.
 */
void dpx_load(struct dpx_t *dpx, char *msg)
{
        strcpy(dpx->buf, msg);
}


/**
 * dpx_read -- read the subscription FIFO into the duplex buffer
 * @dpx: pointer to a duplex structure
 * Returns nothing.
 */
void dpx_read(struct dpx_t *dpx)
{
        fifo_read(dpx->fd_sub, (void *)dpx->buf, (size_t)MIN_PIPESIZE);
}


/**
 * dpx_write -- write the duplex buffer to the publishing FIFO
 * @dpx: pointer to a duplex structure
 * Returns nothing.
 */
void dpx_write(struct dpx_t *dpx)
{
        fifo_write(dpx->fd_pub, (void *)dpx->buf, (size_t)MIN_PIPESIZE);
}

