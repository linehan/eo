#define __MERSENNE__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

#include "suc.h"
#include "meta.h"
#include "parse.h"
#include "../common/io/file.h"
#include "../common/ipc/daemon.h"
#include "../common/ipc/channel.h"

#include "../common/error.h"
#include "../common/util.h"
#include "../common/configfiles.h"
#include "../common/textutils.h"
#include "../common/lib/bloom/bloom.h"


/*
 * We split a script into parts:
 *
 *      suc ~/dir : {@} "tr '[A-Z]' '[a-z]'" : mv {} jpg/
 *
 *      |----------|--------------------------|-----------|
 *     
 *       statement0         statement1         statement2
 *
 *  
 * statement0:
 *
 *      suc <directory>
 *
 * statement1:
 *      
 *      {@} "tr '[A-Z]' '[a-z]'"
 *      ``` ````````````````````
 *       |           |
 *    operator    operand
 *
 * statement2:
 *
 *           operator
 *          /
 *      mv {} jpg/
 *      ``````````
 *       operand 
 *
 *
 *
 * {@} <TRANSFORM()> 
 * is a unary prefix operator. It accepts a single operand in the form of a 
 * shell directive.
 *
 * It will rename an iterated filename according to the result of the 
 * <TRANSFORM()> operation as applied to the original filename.
 *
 * In shell script, the expanded operation might look like:
 *
 *      Given {@} "tr '[A-Z]' '[a-z]'" 
 *
 *      0. Expand 
 *              {@}  --->  FileName.JPG
 *
 *      1. Transform
 *              newname=$(echo FileName.JPG | "tr '[A-Z]' '[a-z]'")
 *
 *      2. Rename
 *              mv FileName.JPG $newname 
 *
 * suc attempts to optimize some of the system calls involved, by using the
 * low-level i/o functions where possible. Instead of mv, for example, {@}
 * uses the rename() function.
 *
 * Generically, given {@} TRANSFORM(filename), the expansion proceeds:
 *
 *      0. Expand    
 *              {@}  --->  <ITERATED FILENAME>
 *
 *      1. Transform 
 *              <ALTERED FILENAME> := TRANSFORM(<ITERATED FILENAME>)
 *
 *      2. Rename    
 *              rename(<ITERATED FILENAME>, <ALTERED FILENAME>)
 * 
 */


/**
 * op_transform
 * ````````````
 * Perform an in-place transform and rename. {@}
 * 
 * @self    : pointer to the operator object.
 * @filename: name of the current file (may be altered).
 * Return   : 0 on success, -1 on failure.
 */
int op_transform(void *self, char *filename)
{
        struct op_t *op = (struct op_t *)self;
        static char new[PATHSIZE];

        bounce(new, PATHSIZE, "echo %s | %s", filename, op->command);

        srename(filename, new);

        slcpy(filename, new, PATHSIZE);

        return 0;
}


/**
 * op_void
 * ```````
 * A do-nothing function stub for when there is no operation. 
 * 
 * @self    : pointer to the operator object.
 * @filename: name of the current file (may be altered).
 * Return   : 0 on success, -1 on failure.
 */
int op_void(void *self, char *filename)
{
        return 0;
}


opf_t OPERATION[]={op_void, op_void, op_void, op_transform };
const char *SYMBOL[]={"__VOID__", "suc", "{}", "{@}"};


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

        new = malloc(sizeof(struct op_t)); 

        for (s=0; s<4; s++) {
                /* If the statement contains SYMBOL[s] */
                if ((tmp = strstr(statement, SYMBOL[s]))) {
                       new->command = trimdup((tmp + strlen(SYMBOL[s])));
                       new->tag     = s;
                       new->op      = OPERATION[s];
                       return new;
                } else {
                        continue;
                }
        }
        /* The statement contained no symbols from the table. */
        new->command = trimdup(symbol_text[VOID]);
        new->tag     = VOID;
        new->op      = NULL;
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

