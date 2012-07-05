#ifndef _PARSER_H 
#define _PARSER_H 


/* Operation function pointer type */
typedef int (*opf_t)(void *self, char **filename);

/* Operator tags */
enum op_tag { VOI, SUC, SUB, FRM, FER, LAT };
static const char *op_name[]={"VOI", "SUC", "SUB", "FRM", "FER", "LAT"};

/* 
 * Operation type
 */
struct op_t {
        enum op_tag tag;
        opf_t op;
        char operand[4096];
};


struct routine_t {
        int n;
        struct op_t **op;
};

struct routine_t *parser_analyzer(const char *input);

#endif
