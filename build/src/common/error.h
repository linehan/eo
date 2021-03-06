#ifndef _ERROR_H
#define _ERROR_H
#include "util.h"
#include <errno.h>
#include <signal.h>

/*
 * Exit the program and print a diagnostic message
 */
#define bye(...)                                                         \
        (VA_NUM_ARGS(__VA_ARGS__) == 1) ? abort_report(__VA_ARGS__, "")  \
                                        : abort_report(__VA_ARGS__)      \

/*
 * Raise a signal and print an error.
 */
#define halt(sig, ...)                                                       \
        (VA_NUM_ARGS(__VA_ARGS__) == 1) ? raise_report(sig, __VA_ARGS__, "") \
                                        : raise_report(sig, __VA_ARGS__)     \

void abort_report(const char *fmt, ...);
void raise_report(sig_t signo, const char *fmt, ...);


typedef void (*sig_handler_t)(int signo);
void sigreg(sig_handler_t handler);


#endif

