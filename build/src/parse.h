#ifndef _PARSER_H 
#define _PARSER_H 

#include "ops.h"

/*
 * Entire routine type 
 */ 
struct routine_t {
        int n;
        struct op_t **op;
};


struct routine_t *parse(char *dir, char *statement);


#endif
