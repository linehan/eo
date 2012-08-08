#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "common/textutils.h"

#include "lex.h"


int lex(char tok) 
{
        switch (tok) {
        case '\n': return END;
        case '{' : return LBR;
        case '}' : return RBR;
        case '[' : return LPR;
        case ']' : return RPR;
        case '$' : return SHL;
        case '@' : return MOV;
        case ';' : return SEM;
        default  : return TXT; 
        }
}


int next_lex(char **look, const char *logic)
{
        static int i=0;
        static size_t len=0;
        int tok=0;

        /* Fresh start */
        if (!i && logic)
                len = strlen(logic);
                
        tok = lex(logic[i++]);
        *look++;

        /* Overflow */
        if (i > len)
                i = 0;
                tok = END;
        }

        /* Reset */
        if (tok == END)
                i = 0;

        return tok;
}


