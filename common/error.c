#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

#define _USE_ERRNO_H

#ifdef USE_ERRNO_H
char *etag[]={ "", "EPERM", "ENOENT", "ESRCH", "EINTR", "EIO", "ENXIO",
                   "E2BIG", "ENOEXEC", "EBADF", "ECHILD", "EAGAIN",
                   "ENOMEM", "EACCES", "EFAULT", "ENOTBLK", "EBUSY",
                   "EEXIST", "EXDEV", "ENODEV", "ENOTDIR", "EISDIR",
                   "EINVAL", "ENFILE", "EMFILE", "ENOTTY", "ETXTBSY",
                   "EFBIG", "ENOSPC", "ESPIPE", "EROFS", "EMLINK",
                   "EPIPE", "EDOM", "ERANGE"
};
char *emsg[]={ 
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





/**
 * abort_report -- clean up and exit, printing a brief diagnostic report
 * @fmt: a printf-style format string
 * @...: the variable argument list to the format string
 */
void abort_report(const char *fmt, ...)
{
        char buf[1000];
        va_list args;

        /* Write formatted output to stream */
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);

        #ifdef USE_ERRNO_H
        if (errno)
                printf("%s (%d): %s\n", etag[errno], errno, emsg[errno]);
        else
                printf("An error has occured. ");
        #endif

        printf("The handler reported: \"%s\"\n", buf);

        exit(1);
}

