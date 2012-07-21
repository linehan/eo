#ifndef _OPERATIONS_H
#define _OPERATIONS_H


typedef int (*opf_t)(char *operand, char **filename);


static const char *SYMBOL[] ={"___","suc","%{" ,"#{" ,"${" ,"@{" ,"->{","{" }; 
static const char *op_name[]={"VOI","SUC","PAT","LOG","SHL","MOV","IMP","SUB"};
enum op_tag                  { VOI , SUC , PAT , LOG , SHL , MOV , IMP , SUB };


/*
 * Single operation type
 */ 
struct op_t {
        enum op_tag tag;
        opf_t op;
        char operand[4096];
};


int op_voi(char *operand, char **filename);
int op_suc(char *operand, char **filename);
int op_sub(char *operand, char **filename);
int op_pat(char *operand, char **filename);
int op_log(char *operand, char **filename);
int op_shl(char *operand, char **filename);
int op_mov(char *operand, char **filename);
int op_imp(char *operand, char **filename);


static opf_t OPERATION[]={
        op_voi, op_suc, op_pat, op_log, op_shl, op_mov, op_imp, op_sub
};


#endif
