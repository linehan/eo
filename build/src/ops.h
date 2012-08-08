#ifndef _OPERATIONS_H
#define _OPERATIONS_H

#include "lex.h"

typedef int (*opf_t)(char *operand, char **filename);

/*
 * Single operation type
 */ 
struct op_t {
        enum op_tag tag;
        opf_t op;
        char operand[4096];
};


int eo_nextfile(char *directory, char **filename);

int op_voi(char *operand, char **filename);
int op_sub(char *operand, char **filename);
int op_pat(char *operand, char **filename);
int op_log(char *operand, char **filename);
int op_shl(char *operand, char **filename);
int op_mov(char *operand, char **filename);
int op_imp(char *operand, char **filename);


static opf_t OP[20];

OP[VOI] = op_voi;
OP[SHL] = op_shl;
OP[SEL] = op_sel;
OP[MOV] = op_mov;

#endif
