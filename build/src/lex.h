#ifndef __LEXERZ
#define __LEXERZ

static const char *SYM[] ={"\n" ,"{",  "}",  "[",  "]",  "::", "$",  "@", ";"}; 
static const char *NAM[] ={"END","LBR","RBR","LPR","RPR","IMP","SHL","MOV","SEM","TXT"};
enum TAG { END , LBR,  RBR,  LPR,  RPR,  IMP , SHL , MOV , SEM , TXT };

#define SUB -3
#define VOI -4
#define NUM_OR_ID -2

int next_lex(const char *logic);

#endif
