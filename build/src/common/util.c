#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "lib/sha256/sph_sha2.h"


/**
 * uctohex -- convert a byte into two hex characters
 * @lower: the lower nibble
 * @upper: the upper nibble
 * @value: the value to decompose
 */
inline void uctohex(char *lower, char *upper, char value)
{
        /* Mask the upper and lower bytes of the value */
        *lower = (value & 0xf);       
        *upper = (value & 0xf0) >> 4; 
        /* Is each nibble a letter or integer? */
        *lower = (*lower > 9) ? ('a' + (*lower - 9)) : (*lower + '0');
        *upper = (*upper > 9) ? ('a' + (*upper - 9)) : (*upper + '0');
}


/**
 * strtohex -- convert a string of bytes into a string of hex characters
 * @dst: the destination buffer
 * @src: the source buffer
 * @len: the size of the source buffer
 */
inline void strtohex(char *dst, char *src, size_t len)
{
        size_t i;
        size_t k;

        for ((i=k=0); (i<len && k<(len*2)-1); (i++, k+=2)) {
                uctohex(&dst[k], &dst[(k+1)], src[i]);
        }
        dst[k] = '\0'; /* Make dst a proper string */
}


/**
 * sha256gen -- return a hex string of the sha256sum
 * @hex : will be filled with the sha256sum. Must be at least 64 bytes
 * @hash: the data used to generate the sha256sum
 */
void sha256gen(char *hex, void *hash)
{
        #define SHA32 32
        sph_sha256_context context;
        char output[SHA32];

        sph_sha256_init(&context);
        sph_sha256(&context, hash, sizeof(hash));
        sph_sha256_close(&context, output);

        strtohex(hex, output, SHA32);
}


/**
 * nsleep -- nanosleep made easy
 */
void nsleep(long nanoseconds)
{
        const struct timespec ts = { .tv_nsec = nanoseconds };
        nanosleep(&ts, NULL);
}


