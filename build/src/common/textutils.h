#ifndef __TEXTUTILS_H
#define __TEXTUTILS_H

void empty(char *str, size_t len);
char *sdup(const char *str);
char *match(const char *haystack, const char *needle);
char *field(const char *string, const char *delimiter);
void pumpf(char **strp, const char *fmt, ...);
void *memchar(void *src_void, unsigned char c, size_t len);
char *trimws(char *str);

void chrswp(char *src, char at, char with, size_t len);

size_t slcpy(char *dst, const char *src, size_t siz);
size_t slcat(char *dst, const char *src, size_t siz);

#define memmem textutils_memmem
void *textutils_memmem(const void *haystack, const void *needle);
#define strstr textutils_strstr 
char *textutils_strstr(const char *h, const char *n);

void trim(char *str);
char *trimdup(char *str);
int ntok(const char *str, const char *tok);

#endif
