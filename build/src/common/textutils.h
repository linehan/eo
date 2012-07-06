#ifndef __TEXTUTILS_H
#define __TEXTUTILS_H

#include <string.h>

/* Initialization ----------------------------------------------------------- */
void szero(char *str);

/* Safe strings ------------------------------------------------------------- */
char  *sdup(const char *str);
char  *sldup(const char *str, size_t max);
size_t slcpy(char *dst, const char *src, size_t siz);
size_t slcat(char *dst, const char *src, size_t siz);
size_t sbif(char *l, char *r, const char *str, const char *tok);

/* String sets -------------------------------------------------------------- */
size_t catenate(char *dest, size_t max, int n, char *strings[]);
//const char *concat(const char *a, const char *b);
char *match(const char *haystack, const char *needle);
char *field(const char *string, const char *delimiter);
int    ntok(const char *str, const char *tok);
void chrswp(char *src, char at, char with, size_t len);

/* Format print ------------------------------------------------------------- */
void pumpf(char **strp, const char *fmt, ...);

/* Whitespace --------------------------------------------------------------- */
size_t trimcpy(char *dst, const char *src);
char *trimws(char *str);

/* Raw memory --------------------------------------------------------------- */
#define memmem textutils_memmem
#define strstr textutils_strstr 
#define memchr textutils_memchr

void *textutils_memmem(const void *haystack, const void *needle);
char *textutils_strstr(const char *haystack, const char *needle);
void *textutils_memchr(const void *src_void, int c, size_t len);

/* Nice macros -------------------------------------------------------------- */

#define STRCMP(a,b) (strcmp((a),(b)) == 0) ? true : false
#define isarg(n, string) (STRCMP(argv[(n)], (string)))
#define ARG(n)           (argv[(n)])


/**
 * concat
 * ``````
 * Return pointer to a static value of 2 concatenated strings.
 * @a: first string (head)
 * @b: second string (tail)
 * Return: pointer to the concateneated string, static. 
 */
static inline const char *concat(const char *a, const char *b)
{
        #define BIG 9000
        static char buffer[BIG];

        slcpy(buffer, a, BIG);
        slcat(buffer, b, BIG);

        return buffer;
}

#endif
