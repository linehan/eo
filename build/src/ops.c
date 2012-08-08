#define __MERSENNE__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>

#include "eo.h"
#include "meta.h"
#include "parse.h"
#include "regex.h"
#include "common/io/file.h"
#include "common/io/dir.h"
#include "common/io/shell.h"
#include "common/ipc/daemon.h"
#include "common/ipc/channel.h"

#include "common/error.h"
#include "common/util.h"
#include "common/configfiles.h"
#include "common/textutils.h"
#include "common/lib/bloom/bloom.h"

/* DEBUG */
//#define DEBUG_BREAK 
#define SHOW_PARSE 
#define SHOW_LOGIC


/******************************************************************************
 * OPERATIONS
 *
 * These functions define operations that are bound to the operator tokens
 * in the symbol table. Each of them conforms to a single prototype, taking
 * two parameters:
 *
 *      1. Pointer to an operation object (struct op_t).
 *      2. Pointer to a mutable filename string.
 *
 * Any static data about the operation to be performed will be contained in
 * the operation object. Things such as which shell command to run, or which
 * directory a file should be moved to, which do not change between 
 * operations.
 *
 * The filename itself may be altered during the operation, so that the next
 * operation will receive a pointer to the filename, etc., and it will be
 * transformed through successive operations.
 *
 ******************************************************************************/


/**
 * eo_nextfile 
 * ```````````
 * Yield filenames from a directory.
 * 
 * @directory: Path to a directory 
 * @filename : Name of the current file (altered by eo_nextfile).
 * Return    : 0 on success, -1 on failure.
 */
int eo_nextfile(char *directory, char **filename)
{
        static DIR *dir;

        /* First invocation */
        if (dir == NULL) {
                make_path_absolute(directory);
                dir = sdopen(directory); // open the directory
        }

        if ((*filename = getfile(dir, F_REG))) {
                return 1;
        } else {
                sdclose(dir);
                return -1;
        }
}




/**
 * op_voi
 * ``````
 * A do-nothing function stub for when there is no operation. 
 * 
 * @operand : operand (can be NULL) 
 * @filename: name of the current file (may be altered).
 * Return   : 0 on success, -1 on failure.
 */
int op_voi(char *operand, char **filename)
{
        return 1;
}


/**
 * op_mov
 * ``````
 * Perform an in-place rename (mv).
 * 
 * @operand : Desired (target) filename or directory of file. 
 * @filename: name of the current file (may be altered).
 * Return   : 1 on success, -1 on failure.
 */
int op_mov(char *operand, char **filename)
{
        static char result[LINESIZE];

        if (bounce(result, LINESIZE, "mv %s %s", *filename, operand)) {
                srename(*filename, result);
                slcpy(*filename, result, PATHSIZE);
                return 1;
        } else {
                printf("%s\n", result);
                return 0;
        }
}


/**
 * op_pat
 * ``````
 * Print the value of the expression on the screen. 
 * 
 * @operand : ? 
 * @filename: name of the current file (may be altered).
 * Return   : 1 on success, -1 on failure.
 */
int op_pat(char *operand, char **filename)
{
        return 1;
}


/**
 * op_log
 * ``````
 * Print the value of the expression on the screen. 
 * 
 * @operand : ? 
 * @filename: name of the current file (may be altered).
 * Return   : 1 on success, -1 on failure.
 */
int op_log(char *operand, char **filename)
{
        printf("%s\n", *filename);

        return 1;
}


/**
 * op_shl
 * ``````
 * @operand : shell command
 * @filename: name of the current file (may be altered).
 * Return   : 1 on success, -1 on failure.
 */
int op_shl(char *operand, char **filename)
{
        return echo("%s", operand) ? 1 : 0;
}


/**
 * op_imp
 * ``````
 * @operand : ? 
 * @filename: name of the current file (may be altered).
 * Return   : 1 on success, -1 on failure.
 */
int op_imp(char *operand, char **filename)
{
        return 1;
}


/**
 * op_sub
 * ``````
 * Conditionally substitute the filename value for the token {}. 
 * 
 * @operand : pattern expression to match each filename against.
 * @filename: name of the current file (may be altered).
 * Return   : 0 on success, -1 on failure.
 *
 * USAGE
 * The token {} can be used empty, in which case every filename will
 * "match" the empty pattern and thus be substituted. It can also be
 * used with some stipulation expression such as {*.jpg}, in which case
 * only those filenames which match the pattern specified will be
 * substituted.
 *
 * The token need not be used on its own, but will expand in whatever
 * statement it is contained within.
 */
int op_sub(char *operand, char **filename)
{
        static bool first = true;
        static char match[PATHSIZE];
        static char lside[LINESIZE];
        static char rside[LINESIZE];
        char *tmp;
        int i;

        if (first) {
                tmp = operand;

                for (i=0; *tmp != '{'; i++, tmp++)
                        lside[i] = *tmp;

                tmp++;

                for (i=0; *tmp != '}'; i++, tmp++)
                        match[i] = *tmp;

                tmp++;

                for (i=0; *tmp != '\0'; i++, tmp++)
                        rside[i] = *tmp;

                first = false;
        }

        if (STREMPTY(match))
                match[0] = '*';

        if (kleene(match, *filename)) {
                echo("%s %s %s", lside, *filename, rside);
        }

        return 1;
}


