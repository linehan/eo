#ifndef __TEXTUTILS_H
#define __TEXTUTILS_H

void empty(char *str, size_t len);
char *sdup(const char *str);
char *match(const char *haystack, const char *needle);
char *field(const char *string, const char *delimiter);
void pumpf(char **strp, const char *fmt, ...);
void *memchar(void *src_void, unsigned char c, size_t len);
void trimws(char *str);

void chrswp(char *src, char at, char with, size_t len);

size_t slcpy(char *dst, const char *src, size_t siz);
size_t slcat(char *dst, const char *src, size_t siz);

void *memem(const void *haystack, const void *needle);

#endif
