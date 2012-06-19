#ifndef _PUMP_ERROR_H
#define _PUMP_ERROR_H
#include "util.h"


#define bye(...) do { if (VA_NUM_ARGS(__VA_ARGS__) == 1)  \
                        abort_report(__VA_ARGS__, ""); \
                   else                                \
                        abort_report(__VA_ARGS__); } while(1) \

#define ASSERT(x) if (!(x)) abort_report("Assert failed: " #x)


void abort_report(const char *fmt, ...);


#ifdef USE_ERRNO_H
static char *etag[]={ "", "EPERM", "ENOENT", "ESRCH", "EINTR", "EIO", "ENXIO",
                          "E2BIG", "ENOEXEC", "EBADF", "ECHILD", "EAGAIN",
                          "ENOMEM", "EACCES", "EFAULT", "ENOTBLK", "EBUSY",
                          "EEXIST", "EXDEV", "ENODEV", "ENOTDIR", "EISDIR",
                          "EINVAL", "ENFILE", "EMFILE", "ENOTTY", "ETXTBSY",
                          "EFBIG", "ENOSPC", "ESPIPE", "EROFS", "EMLINK",
                          "EPIPE", "EDOM", "ERANGE"
};
static char *emsg[]={ 
        "",
        "Operation not permitted", 
        "No such file or directory",
        "No such process",
        "Interrupted system call",
        "I/O error",
        "No such device or address",
        "Argument list too long",
        "Exec format error",
        "Bad file number",
        "No child processes",
        "Try again",
        "Out of memory",
        "Permission denied",
        "Bad address",
        "Block device required",
        "Device or resource busy",
        "File exists",
        "Cross-device link",
        "No such device",
        "Not a directory",
        "Is a directory",
        "Invalid argument",
        "File table overflow",
        "Too many open files",
        "Not a typewriter",
        "Text file busy",
        "File too large",
        "No space left on device",
        "Illegal seek",
        "Read-only file system",
        "Too many links"
        "Broken pipe",
        "Math argument out of domain of func",
        "Math result not representable"
};
#endif /* USE_ERRNO_H */

#endif

