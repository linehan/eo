#ifndef __TEXTUTILS_H
#define __TEXTUTILS_H

void szero(char *str);

char  *sdup(const char *str);
char *sldup(const char *str, size_t max);

size_t slcpy(char *dst, const char *src, size_t siz);
size_t slcat(char *dst, const char *src, size_t siz);

char *match(const char *haystack, const char *needle);
char *field(const char *string, const char *delimiter);
void pumpf(char **strp, const char *fmt, ...);

void *memchar(void *src_void, unsigned char c, size_t len);

size_t trimcpy(char *dst, const char *src);
char *trimws(char *str);

void chrswp(char *src, char at, char with, size_t len);



#define memmem textutils_memmem
void *textutils_memmem(const void *haystack, const void *needle);
#define strstr textutils_strstr 
char *textutils_strstr(const char *h, const char *n);


int ntok(const char *str, const char *tok);

#endif
