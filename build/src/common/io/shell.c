#define USE_ERRNO_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#include "file.h"
#include "../error.h"
#include "../util.h"
#include "../textutils.h"

/****************************************************************************** 
 * PIPES 
 * 
 ******************************************************************************/

/**
 * shell
 * `````
 * Get the result of command from the shell.
 *
 * @buf  : destination buffer
 * @cmd  : command buffer
 * Return: nothing
 */
static inline int shell(char *buf, const char *cmd)
{
        FILE *pipe;
        int retval;

        /* 
         * Write the command buffer to the shell and read
         * the response into the pipe descriptor.
         */
        pipe = popen(cmd, "r"); 

        /* 
         * Scan the contents of the pipe descriptor into
         * the destination buffer.
         */
        if ((fscanf(pipe, "%s", buf)) == 0)
                retval = 0;
        else
                retval = 1;

        fclose(pipe);

        return retval;
}


/**
 * bounce
 * ``````
 * Write the result of a shell command to a buffer.
 *
 * @buf  : destination buffer
 * @max  : size of destination buffer
 * @fmt  : format string
 * @...  : arguments for the format string
 * Return: nothing. 
 */
void bounce(char *buf, size_t max, const char *fmt, ...)
{
        char cmd[LINESIZE];
        va_list args;

        /* Parse the format string into the command buffer */
        va_start(args, fmt);
        vsnprintf(cmd, LINESIZE, fmt, args);
        va_end(args);

        if (!(shell(buf, cmd)))
                slcpy(buf, "", max);
}


/**
 * echo 
 * ````
 * Print the result of a shell command to the stdout of the calling process.
 *
 * @fmt  : format string
 * @...  : arguments for the format string
 * Return: nothing. 
 *
 * NOTE
 * This essentially wraps the bounce() function, but prints the returned
 * buffer contents to stdout, since any echo performed in the bounce pipe
 * will print to the stdout of the forked shell process instead of the
 * caller's process.
 */
void echo(const char *fmt, ...)
{
        char buf[LINESIZE];
        char cmd[LINESIZE];
        va_list args;

        /* Parse the format string into the command buffer */
        va_start(args, fmt);
        vsnprintf(cmd, LINESIZE, fmt, args);
        va_end(args);

        if ((shell(buf, cmd)))
                printf("%s\n", buf);
}


