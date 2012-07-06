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
#include "channel.h"
#include "fifo.h"


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
 * dpx_creat 
 * `````````
 * Create a duplex channel in the filesystem
 *
 * @path : path of the duplex directory
 * @perms: file permissions 
 * Return: nothing.
 */
void dpx_creat(const char *path)
{
        smkdir(path, DIR_PERMS);
        fifo_creat(concat(path, "/sub"), PERMS);
        fifo_creat(concat(path, "/pub"), PERMS);
}


/**
 * dpx_remove 
 * ``````````
 * Destroy named FIFOs for a duplex channel
 *
 * @path : remove the channel at directory 'path'
 * Return: nothing
 */
void dpx_remove(const char *path)
{
        fifo_remove(concat(path, "/sub"));
        fifo_remove(concat(path, "/pub"));
        srmdir(path);
}
 

/**
 * dpx_open 
 * ````````
 * Open a new channel. 
 *
 * @dpx  : pointer to a duplex structure
 * @path : path where FIFOs will be created (path.pub/path.sub)
 * @mode : are we creating the FIFO files as we open?
 * Return: nothing.
 */
void dpx_open(struct dpx_t *dpx, const char *path, int mode)
{
        /* Optionally create the channel files */
        if (NEW_CH(mode))
                dpx_creat(path);

        /* Set the role of the channel terminal */
        dpx->role = CH_ROLE(mode);

        /* Set the path of the channel's disk files */
        dpx->path = sdup(path);

        /* Open the file descriptors in the appropriate order */
        if (dpx->role == PUBLISH) {
                /* Set the publish and subscribe paths */
                dpx->path_pub = sdup(concat(path, "/pub"));
                dpx->path_sub = sdup(concat(path, "/sub"));

                /* Open the publish and subscribe paths */ 
                dpx->fd_sub = fifo_open(dpx->path_sub, O_RDONLY);
                dpx->fd_nub = fifo_open(dpx->path_sub, O_WRONLY);
                dpx->fd_pub = fifo_open(dpx->path_pub, O_WRONLY); // keepalive

        } else if (dpx->role == SUBSCRIBE) {
                /* Set the publish and subscribe paths */ 
                dpx->path_pub = sdup(concat(path, "/sub"));
                dpx->path_sub = sdup(concat(path, "/pub"));

                /* Open the publish and subscribe paths */ 
                dpx->fd_pub = fifo_open(dpx->path_pub, O_WRONLY);
                dpx->fd_sub = fifo_open(dpx->path_sub, O_RDONLY);

        } else {
                bye("open_dpx: Invalid duplex role");
        }
}


/**
 * dpx_close 
 * `````````
 * Close an open channel
 *
 * @dpx  : pointer to a duplex structure
 * Return: nothing.
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



/******************************************************************************
 * CHANNEL TRANSMISSION 
 *
 * dpx_read  -- read the channel into the transmission buffer
 * dpx_write -- write the transmission buffer to the channel 
 * dpx_load  -- load a message into the transmission buffer 
 * dpx_flush -- fill the transmission buffer with 0's 
 * dpx_send  -- load and write the transmission buffer in one call 
 * dpx_form  -- send formatted message
 * dpx_link  -- handshake and exchange PIDs
 * dpx_kill  -- signal the other terminal of a channel
 *
 ******************************************************************************/


/**
 * dpx_read 
 * ````````
 * Read the channel into the transmission buffer 
 *
 * @dpx  : pointer to a duplex structure
 * Return: nothing.
 */
void dpx_read(struct dpx_t *dpx)
{
        fifo_read(dpx->fd_sub, (void *)dpx->buf, (size_t)MIN_PIPESIZE);
}


/**
 * dpx_write 
 * `````````
 * Write the transmission buffer to the channel 
 *
 * @dpx  : pointer to a duplex structure
 * Return: nothing.
 */
void dpx_write(struct dpx_t *dpx)
{
        fifo_write(dpx->fd_pub, (void *)dpx->buf, (size_t)MIN_PIPESIZE);
}


/**
 * dpx_load 
 * ````````
 * Load a message buffer into the transmission buffer 
 *
 * @dpx  : pointer to a duplex structure
 * @msg  : message to be loaded in the duplex structure
 * Return: nothing.
 */
void dpx_load(struct dpx_t *dpx, const char *msg)
{
        slcpy(dpx->buf, msg, MIN_PIPESIZE);
}


/**
 * dpx_flush 
 * `````````
 * Fill the transmission buffer with 0's 
 *
 * @dpx  : pointer to a duplex structure
 * Return: nothing.
 */
void dpx_flush(struct dpx_t *dpx)
{
        szero(dpx->buf);
}


/**
 * dpx_send 
 * ````````
 * Load and write the transmission buffer in one call
 *
 * @dpx  : pointer to a duplex structure
 * @msg  : message to be loaded into the duplex
 * Return: nothing.
 */
void dpx_send(struct dpx_t *dpx, const char *msg)
{
        dpx_load(dpx, msg);
        dpx_write(dpx);
}


/**
 * dpx_sendf
 * `````````
 * Write a format string and arguments to the dpx buffer
 *
 * @dpx: pointer to a duplex structure 
 * @fmt: format string 
 * @...: variadic argument list
 * 
 * USAGE
 * This wraps the vsnprintf function, and the calling convention
 * is consistent with all the other format print functions, e.g.
 *
 *      dpx_sendf(&mydpx, "String %s occurs %d times\n", str, num);
 *
 */
void dpx_sendf(struct dpx_t *dpx, const char *fmt, ...) 
{
        va_list args;

        va_start(args, fmt);
        vsnprintf(dpx->buf, MIN_PIPESIZE, fmt, args);
        va_end(args);

        dpx_write(dpx);
}       


/**
 * dpx_ping
 * ````````
 * Identical to dpx_send, but waits for a response.
 *
 * @dpx  : pointer to a duplex structure
 * @msg  : message to be loaded into the duplex
 * Return: nothing.
 */
void dpx_ping(struct dpx_t *dpx, const char *msg)
{
        dpx_load(dpx, msg);
        dpx_write(dpx);
        dpx_read(dpx);
}


/**
 * dpx_pingf
 * `````````
 * Identical to dpx_sendf, but waits for a response.
 *
 * @dpx: pointer to a duplex structure 
 * @fmt: format string 
 * @...: variadic argument list
 * 
 * USAGE
 * This wraps the vsnprintf function, and the calling convention
 * is consistent with all the other format print functions, e.g.
 *
 *      dpx_sendf(&mydpx, "String %s occurs %d times\n", str, num);
 *
 */
void dpx_pingf(struct dpx_t *dpx, const char *fmt, ...) 
{
        va_list args;

        va_start(args, fmt);
        vsnprintf(dpx->buf, MIN_PIPESIZE, fmt, args);
        va_end(args);

        dpx_write(dpx);
        dpx_read(dpx);
}       


/**
 * dpx_link 
 * ````````
 * Connect, handshake and exchange PIDs.
 *
 * @dpx  : pointer to a duplex structure
 * Return: nothing.
 */
void dpx_link(struct dpx_t *dpx)
{
        pid_t pid;

        /* Exchange PIDs */
        pid = getpid();

        if (dpx->role == PUBLISH) {
                dpx_read(dpx);
                dpx->remote_pid = atoi(dpx->buf);
                dpx_sendf(dpx, "%d", pid);
                dpx_read(dpx);
        } else if (dpx->role == SUBSCRIBE) {
                dpx_sendf(dpx, "%d", pid);
                dpx_read(dpx);
                dpx->remote_pid = atoi(dpx->buf);
                dpx_send(dpx, "ack");
        }
}


/**
 * dpx_kill 
 * ````````
 * Signal the other end of a duplex link using the stored PID
 * 
 * @dpx  : pointer to a duplex structure
 * @signo: signal to be raised
 * Return: nothing.
 */
void dpx_kill(struct dpx_t *dpx, int signo)
{
        kill(dpx->remote_pid, signo);
}

