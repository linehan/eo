#define __MERSENNE__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

#include "suc.h"
#include "meta.h"
#include "parse.h"
#include "../common/io/file.h"
#include "../common/io/dir.h"
#include "../common/ipc/daemon.h"
#include "../common/ipc/channel.h"

#include "../common/error.h"
#include "../common/util.h"
#include "../common/configfiles.h"
#include "../common/textutils.h"
#include "../common/lib/bloom/bloom.h"



/* Operation functions */
int op_voi(void *self, char **filename);
int op_frm(void *self, char **filename);
int op_suc(void *self, char **filename);
int op_lat(void *self, char **filename);

/* Symbol table */
opf_t OPERATION[]={op_voi, op_suc, op_voi, op_frm, op_voi, op_lat};
const char *SYMBOL[]={"__VOID__", "suc", "__VOID__", "{@}", "__VOID__", "{}"};


/**
 * op_suc
 * ``````
 * Yield the next filename from the pool.
 * 
 * @self    : pointer to the operator object.
 * @filename: name of the current file (may be altered).
 * Return   : 0 on success, -1 on failure.
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

        if ((*filename = (getfile(dir, F_REG)))) {
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
}






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
        int s;

        new = calloc(1, sizeof(struct op_t)); 

        for (s=0; s<6; s++) {
                /* If the statement contains SYMBOL[s] */
                if ((tmp = strstr(statement, SYMBOL[s]))) {
                        trimcpy(new->operand, (tmp + strlen(SYMBOL[s])));
                        new->tag = s;
                        new->op  = OPERATION[s];
                        return new;
                } else {
                        continue;
                }
        }
        /* The statement contained no symbols from the table. */
        slcpy(new->operand, op_name[VOI], 4096); 
        new->tag = VOI;
        new->op  = NULL;
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

