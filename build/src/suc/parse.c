#define __MERSENNE__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>

#include "suc.h"
#include "meta.h"
#include "parse.h"
#include "regex.h"
#include "../common/io/file.h"
#include "../common/io/dir.h"
#include "../common/io/shell.h"
#include "../common/ipc/daemon.h"
#include "../common/ipc/channel.h"

#include "../common/error.h"
#include "../common/util.h"
#include "../common/configfiles.h"
#include "../common/textutils.h"
#include "../common/lib/bloom/bloom.h"


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

/* Operation functions */
int op_voi(void *self, char **filename);
int op_suc(void *self, char **filename);
int op_sub(void *self, char **filename);
int op_frm(void *self, char **filename);
int op_lat(void *self, char **filename);
int op_fmt(void *self, char **filename);


/* Symbol table */
opf_t OPERATION[]={op_voi, op_suc, op_sub, op_frm, op_voi, op_lat, op_fmt};
const char *SYMBOL[]={"__VOID__", "suc", "{}", "{@}", "__VOID__", "{#}", "{%}"};


/**
 * op_voi
 * ``````
 * A do-nothing function stub for when there is no operation. 
 * 
 * @self    : pointer to the operator object.
 * @filename: name of the current file (may be altered).
 * Return   : 0 on success, -1 on failure.
 */
int op_voi(void *self, char **filename)
{
        return 1;
}


/**
 * op_sub
 * ``````
 * Substitute the current value of filename in the operand, and run in shell. 
 * 
 * @self    : pointer to the operator object.
 * @filename: name of the current file (may be altered).
 * Return   : 0 on success, -1 on failure.
 */
int op_sub(void *self, char **filename)
{
        #define SUB_TOKEN "{}"
        static bool first = true;
        static char lside[LINESIZE];
        static char rside[LINESIZE];

        struct op_t *op = (struct op_t *)self;

        if (first) {
                sbif(lside, rside, op->operand, SUB_TOKEN);
                first = false;
        }

        echo("%s %s %s", lside, *filename, rside);

        return 1;
}


/**
 * op_suc
 * ``````
 * Yield the next filename from the pool.
 * 
 * @self    : pointer to the operator object.
 * @filename: name of the current file (may be altered).
 * Return   : 0 on success, -1 on failure.
 *
 * NOTE
 * The static operand is the path of the directory to be opened. 
 */
int op_suc(void *self, char **filename)
{
        struct op_t *op = (struct op_t *)self;
        static DIR *dir;

        /* First invocation */
        if (dir == NULL) {
                make_path_absolute(op->operand);
                dir = sdopen(op->operand); // open the directory
        }

        if ((*filename = getfile(dir, F_REG))) {
                return 1;
        } else {
                sdclose(dir);
                return -1;
        }
}


/**
 * op_frm 
 * ``````
 * Perform an in-place transform and rename. {@}
 * 
 * @self    : pointer to the operator object.
 * @filename: name of the current file (may be altered).
 * Return   : 0 on success, -1 on failure.
 *
 * NOTE
 * The static operand is the shell command to be used in the transform.
 */
int op_frm(void *self, char **filename)
{
        struct op_t *op = (struct op_t *)self;
        static char new[PATHSIZE];

        bounce(new, PATHSIZE, "echo %s | %s", *filename, op->operand);

        srename(*filename, new);

        slcpy(*filename, new, PATHSIZE);

        return 1;
}


/**
 * op_lat
 * ``````
 * Print the value of the expression on the screen. 
 * 
 * @self    : pointer to the operator object.
 * @filename: name of the current file (may be altered).
 * Return   : 0 on success, -1 on failure.
 */
int op_lat(void *self, char **filename)
{
        printf("%s\n", *filename);

        return 1;
}


/******************************************************************************
 * INTERPRETER 
 *
 * The interpreter operates in two steps. The parser-analyzer splits the
 * code into units delimited by the colon character ':'. Each of these
 * blocks is given to the semantic analyzer, which builds an operation
 * object from the content of the unit. The semantic analyzer returns a
 * pointer to this object, which is added to a vector maintained by the
 * parser analyzer, and eventually returned to the caller.
 *
 ******************************************************************************/


/**
 * semantic_analyzer 
 * `````````````````
 * Semantic analyzer: detect and couple operators with logic and operands.
 *
 * @statement: A code statement which may or may not contain an operator. 
 * Return    : Allocated semantic unit in the form of a struct op_t.
 */
struct op_t *semantic_analyzer(const char *statement)
{
        struct op_t *new; 
        char *tmp;
        char *arg;
        int s;

        new = calloc(1, sizeof(struct op_t)); 

        /* Search for tokens in the logic statement. */
        for (s=0; s<7; s++) {
                if ((tmp = strstr(statement, SYMBOL[s]))) {
                        switch (new->tag = s) {
                        case FMT:
                                trimcpy(new->operand, (tmp+strlen(SYMBOL[s])));
                                break;
                        case SUB:
                                trimcpy(new->operand, statement);
                                break;
                        case SUC:
                                trimcpy(new->operand, (tmp+strlen(SYMBOL[s])));
                                make_path_absolute(new->operand);
                                break;
                        default:
                                trimcpy(new->operand, (tmp+strlen(SYMBOL[s])));
                                break;
                        }
                        new->op  = OPERATION[s];
                        return new;
                } else {
                        continue;
                }
        }
        /* The statement contained no symbols from the table. */
        slcpy(new->operand, op_name[VOI], 4096); 
        new->tag = VOI;
        new->op  = OPERATION[VOI];

        return new;
}


/**
 * parser_analyzer 
 * ```````````````
 * Break an input string into statements and expressions. 
 *
 * @input: concatenated input string.
 * Return: routine struct.
 */
struct routine_t *parser_analyzer(const char *input)
{
        #define DELIM ":"
        struct routine_t *new;
        char *code;
        char *statement;
        int i = 0;

        code = sdup(input);

        new     = malloc(sizeof(struct routine_t));
        new->n  = (ntok(input, DELIM))+1; // fencepost
        new->op = malloc(new->n * sizeof(struct op_t *));

        for (statement = strtok(code, DELIM); 
             statement; 
             statement = strtok(NULL, DELIM))
        {
                new->op[i++] = semantic_analyzer(statement);
        }

        free(code);

        return new;
}


/**
 * parse
 * `````
 * Parses the content of the shell arguments into actionable logic.
 *
 * @argc: number of shell arguments
 * @argv: vector of argument strings
 * Return: a routine struct that will guide processing.
 */
struct routine_t *parse(int argc, char *argv[])
{
        static char logic[LINESIZE];
        struct routine_t *new;

        catenate(logic, LINESIZE, argc, argv); 
        new = parser_analyzer(logic);

        return new;
}




