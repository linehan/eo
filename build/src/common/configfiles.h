#ifndef _PUMP_CONFIG_FILES_H
#define _PUMP_CONFIG_FILES_H

/******************************************************************************
 * FILES AND DIRECTORIES 
 ******************************************************************************/

/*
 * File permissions
 *
 * When a daemon process is forked, its process group is dissociated
 * from process group of its parent. It is important that if files 
 * are created by a parent process, they are given permissions that
 * will allow the daemon to access them as well, since from the 
 * kernel's perspective, the daemon's process bears no relation to the 
 * owner/creator of the files.
 *
 * Specifically, we want to ensure that processes in the "others"
 * triplet will have read and write access. For directories, this
 * means having execute access, because "entering" a directory is
 * equivalent to executing a file of type "directory". 
 *
 *      (u)ser   process which created file
 *      (g)roup  processes sharing the owner's group
 *      (o)ther  any other process
 *
 *        u   g   o 
 *      ======================================================
 *      -rwx rwx rwx      full permissions
 *      -rw- r-- r--      default permissions for files
 *      -rw- rw- rw-      desired permissions for files
 *      drwx r-- ---      default permissions for directories
 *      drwx r-- r-x      desired permissions for directories
 *      =======================================================
 */
#define CFG_PERMS ((S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
#define DIR_PERMS ((S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH))
#define PERMS (0666)


/*
 * Home directory
 *
 * The configuration folder will be placed in the home directory
 * of the user who the process is associated with. See gethome()
 * in ../common/file.c.
 */
#define HOME_DIR  (gethome())


/*
 * Configuration files on disk
 *
 * The pump daemon maintains a number of files, all of which are
 * stored in a hidden directory, CFG_STEM, which resides in the
 * directory identified by CFG_PATH. 
 *
 * CFG_NAME     Name of the hidden directory
 * CFG_PATH     Path of the hidden directory
 *
 */
#define CFG_STEM  ".pump"
#define CFG_PATH  (CONCAT(HOME_DIR, "/"CFG_STEM"/"))


/*
 * PID file
 *
 * In order to signal and stat the daemon, a client needs to know 
 * the pid (process id) of the daemon process. When the daemon is
 * started, it writes this number to the pid file.
 */
#define PID_NAME  "pumpd.pid"
#define PID_PATH  (CONCAT(CFG_PATH, PID_NAME))


/*
 * FIFO files
 *
 * Communication between the daemon and clients is performed via
 * FIFO files (named pipes). A number of these files may need to
 * be maintained, depending on the number of clients and the amount
 * of message multiplexing. 
 *
 * FIFOs which carry messages *to* the server are marked with the 
 * extension ".sub", while those carrying messages *from* the server 
 * are marked with the extension ".pub". 
 *
 * These extensions are automatically appended to the path supplied 
 * to the dpx_creat() function (see daemon.c).
 */
#define FIFO_NAME  "fifo"
#define FIFO_PATH     (CONCAT(CFG_PATH, FIFO_NAME))
#define CHANNEL(stem) (CONCAT(CFG_PATH, stem))

#endif


