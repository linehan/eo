#ifndef _PARSER_H 
#define _PARSER_H 


typedef int (*opf_t)(void *self, char **filename);

static const char *op_name[]={"VOI", "SUC", "SUB", "FRM", "FER", "LAT"};
enum op_tag                  { VOI,   SUC,   SUB,   FRM,   FER,   LAT };


/*
 * Single operation type
 */ 
struct op_t {
        enum op_tag tag;
        opf_t op;
        char operand[4096];
};


/*
 * Entire routine type 
 */ 
struct routine_t {
        int n;
        struct op_t **op;
};


struct routine_t *parse(int argc, char *argv[]);


#endif
