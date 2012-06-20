#ifndef __TEXTUTILS_H
#define __TEXTUTILS_H

void bwipe(char *str);
char *bdup(const char *str);
char *match(const char *haystack, const char *needle);
char *field(const char *string, const char *delimiter);
void pumpf(char **strp, const char *fmt, ...);
void *memchar(void *src_void, unsigned char c, size_t len);
void terminate(char *src, char chr, size_t len);
void trimws(char *str);

#endif
