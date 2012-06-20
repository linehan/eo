/* 
 * textutils.c -- byte-oriented character and string routines.
 *
 * Copyright (C) 2012 Jason Linehan 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, 
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>


/**
 * bwipe -- given a character buffer, set the contents to '\0'
 * @str : pointer to a character buffer
 * @len : size of the character buffer
 */
void bwipe(char *str)
{
        memset(str, '\0', strlen(str));
}


/**
 * bdup -- copy *str to a newly-alloc'd buffer, and return a pointer to it 
 *
 * @str: pointer to a '\0'-terminated char string
 *  RET: pointer to a copy of *str, else NULL.
 */
char *bdup(const char *str)
{
        char *copy;
        size_t len;

        len  = strlen(str) + 1;
        copy = malloc(len);

        return copy ? memcpy(copy, str, len) : NULL;
}


/**
 * match -- locate first occurance of string 'needle' in string 'haystack'
 * @haystack: the string being searched for a match
 * @needle  : the pattern being matched in 'haystack'
 */
char *match(const char *haystack, const char *needle)
{
        size_t len_haystack;
        size_t len_needle;

        if (!needle || !haystack)
                return NULL;

        len_haystack = strlen(haystack);
        len_needle   = strlen(needle);

        /* Needle can't be larger than haystack */
        if (len_needle > len_haystack)
                return NULL;

        return memmem(haystack, len_haystack, needle, len_needle);
}


/**
 * field -- return pointer to a delimited substring (not including delimiter)
 * @str  : the string being matched against
 * @delim: the delimiter to be searched for
 */
char *field(const char *string, const char *delimiter)
{
        size_t offset;
        char *frame;

        if (!string || !delimiter) 
                return NULL;

        if (frame = match(string, delimiter), !frame)
                return NULL;

        offset = strlen(delimiter);

        return &frame[offset];
}


/**
 * pumpf -- write a formatted character string into an auto-allocated buffer
 * @strp : pointer to a character buffer (will be allocated)
 * @fmt  : format string
 * @...  : format string arguments
 * @ret  : length of the formatted string at *strp
 */
void pumpf(char **strp, const char *fmt, ...) 
{
        va_list args;
        size_t len;
        FILE *stream;

        /* Open a new FILE stream. *strp will be dynamically allocated to
         * contain characters written to the stream, and len will reflect
         * these changes. See man(3) open_memstream. */
        stream = open_memstream(strp, &len);

        if (!stream)
        /* Unable to open FILE stream */
                return;

        /* Write formatted output to stream */
        va_start(args, fmt);
        vfprintf(stream, fmt, args);
        va_end(args);

        fflush(stream);
        fclose(stream);
}       


/**
 * Macros for optimizing the newlib memchr implementation (below)
 */
#define LONGALIGNED(X)    ((long)X & (sizeof(long) - 1))
#define LONGBYTES         (sizeof(long))
#define USE_BYTEWISE(len) ((len) < LONGBYTES)
/* 32- and 64-bit magic numbers for detecting NUL characters. */
#define LONGMAX32 2147483647L
#define LONGMAX64 9223372036854775807L
#define LOBITS32  0x01010101 
#define HIBITS32  0x80808080
#define LOBITS64  0x0101010101010101
#define HIBITS64  0x8080808080808080
/* DETECTNUL expands to nonzero if X (long int) contains a NUL character. */
#if LONG_MAX == LONGMAX32 
        #define DETECTNUL(X) (((X) - LOBITS32) & ~(X) & HIBITS32)
#elif LONG_MAX == LONGMAX64 
        #define DETECTNUL(X) (((X) - LOBITS64) & ~(X) & HIBITS64)
#else
        #error memchar: long int is neither a 32bit nor a 64bit value
#endif
/* Expands to nonzero if (long)X contains byte used to fill (long)MASK. */
#define DETECTCHAR(X,MASK) (DETECTNUL(X ^ MASK))


/**
 * memchar -- search memory starting at src for character 'c'
 * If 'c' is found within 'len' characters of 'src', a pointer
 * to the character is returned. Otherwise, NULL is returned.
 */
void *memchar(void *src_void, unsigned char c, size_t len)
{
        const unsigned char *src;
        unsigned char d;
        
        src = (const unsigned char *)src_void;
        d   = c;

        #if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
        unsigned long *asrc;
        unsigned long  mask;
        int i;

        while (LONGALIGNED(src)) {
                if (!len--)
                        return NULL;
                if (*src == d)
                        return (void *)src;
                src++;
        }
        /* 
         * If we get this far, we know that length is large and src is
         * word-aligned. 
         */
        if (!USE_BYTEWISE(len)) {
                /* 
                 * The fast code reads the source one word at a time 
                 * and only performs the bytewise search on word-sized 
                 * segments if they contain the search character, which 
                 * is detected by XOR-ing the word-sized segment with a 
                 * word-sized block of the search character and then 
                 * detecting for the presence of NUL in the result.  
                 */
                asrc = (unsigned long *)src;
                mask = ((d << 8) | d);
                mask = ((mask << 16) | mask);

                for (i=32; i<8*LONGBYTES; i<<=1) {
                        mask = ((mask << i) | mask);
                }

                while (len >= LONGBYTES) {
                        if (DETECTCHAR(*asrc, mask))
                                break;
                        len -= LONGBYTES;
                        asrc++;
                }
                /* 
                 * If there are fewer than LONGBYTES characters left,
                 * we decay to the bytewise loop.
                 */
                src = (unsigned char *)asrc;
        }
        #endif /* !PREFER_SIZE_OVER_SPEED */

        while (len--) {
                if (*src == d)
                        return (void *)src;
                src++;
        }
        return NULL;
}


/**
 * terminate -- replace first instance of byte 'c' in string with NUL ('\0') 
 * @str: string to be truncated
 * @chr: byte to be searched for
 * @len: length of string 'str' 
 */
void terminate(char *src, char chr, size_t len)
{
        char *tmp;

        tmp = (char *)memchar(src, chr, len);

        if (tmp != NULL && *tmp == chr)
                *tmp = '\0';
}


char *trimws(char *str)
{
        char *end;

        /* Trim leading space */
        while (isspace(*str)) 
                str++;

        if (*str == 0) // All spaces?
                return str;

        /* Trim trailing space */
        end = str + strlen(str) - 1;
        while (end > str && isspace(*end)) 
                end--;

        /* Write new null terminator */
        *(end+1) = 0;

        return str;
}
