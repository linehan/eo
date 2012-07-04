#ifndef _PARSE_H
#define _PARSE_H


struct parse_t {
        int n;
        char **node;
};

enum symbol_t { VOID, SUC, SUBSTITUTE, TRANSFORM };
static const char *symbol_text[] = {"VOID", "SUC", "SUBSTITUTE", "TRANSFORM"};

typedef int (*opf_t)(void *self, char *filename);

struct op_t {
        opf_t op;
        enum symbol_t tag;
        char *command;
};


struct routine_t {
        int n;
        struct op_t **op;
};



struct routine_t *parse(const char *input);
char *suc_mvinline(const char *filename, const char *command);
void suc_at(const char *segment);

struct op_t *make_operation(const char *token);

#endif
