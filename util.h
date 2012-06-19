
/**
 * VA_NUM_ARGS
 * Counts the number of VA_ARGS by means of an intense and heathen magic,
 * the particulars of which are not to be uttered here.
 */ 
#define VA_NUM_ARGS(...) \
        VA_NUM_ARGS_IMPL(__VA_ARGS__, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, \
                                      53, 52, 51, 50, 49, 48, 47, 46, 45, 44, \
                                      43, 42, 41, 40, 39, 38, 37, 36, 35, 34, \
                                      33, 32, 31, 30, 29, 28, 27, 26, 25, 24, \
                                      23, 22, 21, 20, 19, 18, 17, 16, 15, 14, \
                                      13, 12, 11, 10,  9,  8,  7,  6,  5,  4, \
                                       3,  2,  1)

#define VA_NUM_ARGS_IMPL( _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, _10, \
                         _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                         _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
                         _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
                         _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
                         _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
                         _61, _62, _63, N, ...) N


#ifdef USE_ERRNO_H
static char *etag[]={
        "",
        "EPERM",
        "ENOENT",
        "ESRCH",
        "EINTR",
        "EIO",
        "ENXIO",
        "E2BIG",
        "ENOEXEC",
        "EBADF",
        "ECHILD",
        "EAGAIN",
        "ENOMEM",
        "EACCES",
        "EFAULT",
        "ENOTBLK",
        "EBUSY",
        "EEXIST",
        "EXDEV",
        "ENODEV",
        "ENOTDIR",
        "EISDIR",
        "EINVAL",
        "ENFILE",
        "EMFILE",
        "ENOTTY",
        "ETXTBSY",
        "EFBIG",
        "ENOSPC",
        "ESPIPE",
        "EROFS",
        "EMLINK",
        "EPIPE",
        "EDOM",
        "ERANGE"
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
#endif

