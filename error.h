#ifndef _PUMP_ERROR_H
#define _PUMP_ERROR_H
#include "util.h"

/*
 * Exit the program and print a diagnostic message
 */
#define bye(...)                                       \
        do {                                           \
                if (VA_NUM_ARGS(__VA_ARGS__) == 1)     \
                        abort_report(__VA_ARGS__, ""); \
                else                                   \
                        abort_report(__VA_ARGS__);     \
        } while(1)


void abort_report(const char *fmt, ...);




#endif

