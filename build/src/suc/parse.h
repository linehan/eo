#ifndef _PARSE_H
#define _PARSE_H


struct parse_t {
        int n;
        char **node;
};


struct parse_t *parse(const char *input);

#endif
