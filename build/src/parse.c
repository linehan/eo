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
#define DEBUG_BREAK 
#define SHOW_PARSE 
#define SHOW_LOGIC


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
        for (s=0; s<8; s++) {
                if ((tmp = strstr(statement, SYMBOL[s]))) {
                        switch (new->tag = s) {
                        case SUB:
                                trimcpy(new->operand, statement);
                                break;
                        case SUC:
                                new->operand[strlen(new->operand)-1] = '\0'; // remove trailing }
                                trimcpy(new->operand, (tmp+strlen(SYMBOL[s])));
                                make_path_absolute(new->operand);
                                break;
                        default:
                                new->operand[strlen(new->operand)-1] = '\0'; // remove trailing }
                                trimcpy(new->operand, (tmp+strlen(SYMBOL[s])));
                                break;
                        }
                        new->op  = OPERATION[s];
                        #if defined(SHOW_PARSE)
                        printf("%d (%s): %s\n", s, op_name[s], new->operand);
                        #endif

                        return new;
                } else {
                        continue;
                }
        }
        /* The statement contained no symbols from the table. */
        slcpy(new->operand, op_name[VOI], 4096); 
        new->tag = VOI;
        new->op  = OPERATION[VOI];
        #if defined(SHOW_PARSE)
        printf("%d (%s): %s\n", VOI, op_name[VOI], new->operand);
        #endif

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
struct routine_t *parse(char *dir, char *statement)
{
        static char logic[LINESIZE];
        struct routine_t *new;

        slcpy(logic, statement, LINESIZE); 

        #if defined(SHOW_LOGIC)
        printf("LOGIC: %s\n\n", logic);
        #endif

        new = parser_analyzer(logic);

        #if defined (DEBUG_BREAK)
        exit(1);
        #endif

        return new;
}


