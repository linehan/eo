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
#include <stdbool.h>

#include "textutils.h"


/**
 * szero 
 * `````
 * Given a character buffer, set the contents to '\0'.
 *
 * @str  : pointer to a byte buffer
 * Return: nothing.
 */
void szero(char *str)
{
        memset(str, '\0', strlen(str));
}


/**
 * sdup 
 * ````
 * Copy *str to a newly-alloc'd buffer, and return a pointer to it.
 *
 * @str  : pointer to a '\0'-terminated char string
 * Return: pointer to a copy of *str, else NULL.
 */
char *sdup(const char *str)
{
        char *copy;
        size_t len;

        len  = strlen(str) + 1;
        copy = malloc(len);

        return copy ? memcpy(copy, str, len) : NULL;
}


/**
 * sldup
 * `````
 * Copy *str to a newly-alloc'd buffer of size len, and return a pointer to it.
 *
 * @str  : pointer to a '\0'-terminated char string
 * @len  : size of buffer (including '\0')
 * Return: pointer to a copy of *str, else NULL.
 */
char *sldup(const char *str, size_t max)
{
        char *copy;
        size_t len;
        size_t end;

        len = strlen(str) + 1;
        len = (len > max) ? max : len; // lesser of two weevils
        end = len - 1;

        if (!(copy = calloc(1, len)))
                return NULL;

        copy[end] = '\0';

        return memcpy(copy, str, end);
}


/**
 * slcpy
 * `````
 * Writes at most len characters from src to dst.
 *
 * @dst  : destination buffer
 * @src  : source buffer
 * @len  : length of source buffer
 * Return: number of bytes written.
 */
size_t slcpy(char *dst, const char *src, size_t siz)
{
        const char *s;
        char *d;
        size_t n;

        d = dst;
        s = src;
        n = siz;

        /* Copy as many bytes from src as will fit in dst */
        if (n != 0) {
                while (--n != 0) {
                        if ((*d++ = *s++) == '\0')
                                break;
                }
        }
        /* 
         * Not enough room in dst, add NUL 
         * and traverse the rest of src 
         */
        if (n == 0) {
                if (siz != 0)
                        *d = '\0'; /* NUL-terminate dst */
                while (*s++)
                ;
        }
        return(s - src - 1); /* count does not include NUL */
}


/**
 * slcat
 * `````
 * Concatenates src and dst in dst.
 *
 * @dst  : destination buffer
 * @src  : source buffer
 * @siz  : size of source buffer
 * Return: Number of bytes concatenated
 */
size_t slcat(char *dst, const char *src, size_t siz)
{
        char *d;
        const char *s;
        size_t n;
        size_t dlen;

        d = dst;
        s = src;
        n = siz;

        /* 
         * Find the end of dst and adjust bytes 
         * left, but don't go past end 
         */
        while (n--!=0 && *d!='\0')
                d++;

        dlen = d - dst;
        n    = siz - dlen;

        if (n == 0)
                return(dlen + strlen(s));

        while (*s != '\0') {
                if (n != 1) {
                        *d++ = *s;
                        n--;
                }
                s++;
        }
        *d = '\0';

        return (dlen + (s - src)); /* count does not include NUL */
}


/**
 * match 
 * `````
 * Locate first occurance of string 'needle' in string 'haystack'
 *
 * @haystack: the string being searched for a match
 * @needle  : the pattern being matched in 'haystack'
 * Return   : The first occurance of 'needle'
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

        return memmem(haystack, needle);
}


/**
 * field 
 * `````
 * Return pointer to a delimited substring (not including delimiter)
 *
 * @str  : the string being matched against
 * @delim: the delimiter to be searched for
 * Return: pointer to the start of the substring
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
 * pumpf 
 * `````
 * Write a formatted character string into an auto-allocated buffer
 *
 * @strp : pointer to a character buffer (will be allocated)
 * @fmt  : format string
 * @...  : format string arguments
 * Return: length of the formatted string at *strp
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


/******************************************************************************/
#define LONGALIGNED(X)    ((long)X & (sizeof(long) - 1))
#define LONGBYTES         (sizeof(long))
#define USE_BYTEWISE(len) ((len) < LONGBYTES)

/* NUL expands to nonzero if X (long int) contains '\0' */
#if LONG_MAX == 2147483647L 
#define NUL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)
#elif LONG_MAX == 9223372036854775807L 
#define NUL(X) (((X) - 0x0101010101010101) & ~(X) & 0x8080808080808080)
#else
#error memchar: long int is neither a 32bit nor a 64bit value
#endif

#define DETECTCHAR(X,MASK) (NUL(X ^ MASK)) /* nonzero if X contains MASK. */


/**
 * textutils_memchr
 * ````````````````
 * Search memory starting at src for character 'c'
 * If 'c' is found within 'len' characters of 'src', a pointer
 * to the character is returned. Otherwise, NULL is returned.
 */
void *textutils_memchr(const void *src_void, int c, size_t len)
{
        const int *src = (const int *)src_void;
        int d = c;

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
                src = (int *)asrc;
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
 * chrswp -- replace (swap) the first occurence of a char byte with another 
 * @src : memory area to be searched 
 * @at  : char byte to be searched for in 'src'
 * @with: char byte which will overwrite the first occurence of 'at' 
 * @len : maximum length to search
 */
void chrswp(char *src, char at, char with, size_t len)
{
        char *sub;
        if ((sub = (char *)memchr(src, at, len)), sub!=NULL && *sub==at)
                *sub = with;
}

char *trimws(char *str)
{
        char *end;

        /* Trim leading space */
        while (isspace(*str)) 
                str++;

        /* Check for empty string */
        if (*str == '\0')
                return str;

        /* Trim trailing space */
        end = str + strlen(str) - 1;
        while (end > str && isspace(*end)) 
                end--;

        /* Write new NUL terminator */
        *(end+1) = '\0';

        return str;
}


/* 
 * Copyright (C) 1991,92,93,94,96,97,98,2000,2004,2007 Free Software Foundation, Inc.
 *  This file is part of the GNU C Library.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  
 */

#ifndef _LIBC
# define __builtin_expect(expr, val)   (expr)
#endif


/* Return the first occurrence of NEEDLE in HAYSTACK.  */
void *textutils_memmem(const void *haystack, const void *needle)
{
        const char *begin;
        const char *final;
        size_t len_haystack;
        size_t len_needle;

        len_haystack = strlen((const char*)haystack);
        len_needle   = strlen((const char*)needle);
       
        final = (const char *)haystack + (len_haystack - len_needle);

        /* The first occurrence of the empty string is deemed to occur at
        the beginning of the string.  */
        if (len_needle == 0)
                return (void *) haystack;

        /* Sanity check, otherwise the loop might search through the whole
        memory.  */
        if (__builtin_expect (len_haystack < len_needle, 0))
                return NULL;

        for (begin = (const char *)haystack; begin <= final; ++begin) {
                if (begin[0] == ((const char *) needle)[0]
                && !memcmp((const void *)&begin[1], (const void *)((const char *)needle+1), len_needle-1))
                        return (void *)begin;
        }

        return NULL;
}

char *textutils_strstr(const char *h, const char *n)
{
        const char *begin;
        const char *end;
        size_t hlen;
        size_t nlen;

        hlen = strlen(h);
        nlen = strlen(n);
       
        end = h + (hlen - nlen);

        /* 
         * The first occurrence of the empty string is deemed to occur at
         * the beginning of the string.  
         */
        if (nlen == 0)
                return (char *)h;

        /* 
         * Sanity check, otherwise the loop might search through the whole
         * memory.  
         */
        if (__builtin_expect (hlen < nlen, 0))
                return NULL;

        for (begin=h; begin<=end; ++begin) {
                if (begin[0] == n[0]
                && !memcmp((const void *)(begin+1),(const void *)(n+1),nlen-1))
                        return (char *)begin;
        }
        return NULL;
}


/**
 * sbif
 * ````
 * Bifurcate a string at into two substrings if a token is found.
 *
 * @l    : destination of left side of string.
 * @r    : destination of right side of string.
 * @str  : original string.
 * @tok  : token to split at.
 * Return: nothing.
 */
size_t sbif(char *l, char *r, const char *str, const char *tok)
{
        const char *cur;
        char *cpy;
        size_t t_len;
        size_t s_len;

        s_len = strlen(str);
        t_len = strlen(tok);
       
        if (t_len == 0)
                return -1;

        if (__builtin_expect (s_len < t_len, 0))
                return -1;

        cpy = l; // Start with the left string.

        for (cur=str; *cur!='\0'; cur++) {
                if (cur[0]==tok[0] && !memcmp((cur+1), (tok+1), (t_len-1))) {
                       *cpy  = '\0';  // NUL-terminate left string.
                        cpy  = r;     // We copy the right side now
                        cur += t_len; // Move cursor past the token
                }
               *cpy = *cur;
                cpy++;
        }
        *cpy = '\0'; // NUL-terminate right string.


        return 1;
}



size_t catenate(char *dest, size_t max, int n, char *strings[])
{
        size_t len = 0;
        int i;

        for (i=0; i<n; i++) {
                len += slcat(dest, strings[i], max);
                len += slcat(dest, " ", max);
        }
        return len;
}


size_t tonext(char *str, char tok)
{
        char *match;
        size_t len;
       
        len = strlen(str);

        match = (char *)memchr(str, tok, len);

        return len - strlen(match);
}



bool is_ws(char c) {
        switch (c) {
        case ' ':
        case '\n':
        case '\t':
        case '\f':
        case '\r':
                return true;
        default:
                return false;
        }
}


/**
 * tail 
 * ````
 * Return pointer to the last character of a string (not including newline).
 */
char *tail(char *string)
{
        return string + strlen(string)-1;
}


/**
 * trimcpy
 * ```````
 * Trim leading/trailing whitespace at src and copy result to dst
 * 
 * @dst  : destination buffer (must be allocated)
 * @src  : source buffer
 * Return: number of bytes written to dst.
 */
size_t trimcpy(char *dst, const char *src)
{
        const char *end;

        /* Leading */
        while (isspace(*src))
                src++; 

        /* All spaces */
        if (*src == 0) 
                return 0;

        /* Trailing */
        end = src + strlen(src) - 1;

        while (end > src && isspace(*end)) 
                end--;
        end++;

        return slcpy(dst, src, (end-src)+1); // slcpy adds NUL
}


int ntok(const char *str, const char *tok)
{
        size_t toklen;
        char *sub;

        int count=0;

        toklen = strlen(tok);

        for (sub  = (char *)memmem(str, tok);
             sub != NULL;
             sub  = (char *)memmem((sub+toklen), tok))
        {
                count++;
        }

        return count;
}

