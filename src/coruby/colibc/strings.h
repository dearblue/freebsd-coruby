#ifndef COLIBC_STRINGS_H__
#define COLIBC_STRINGS_H__

// for /usr/src/lib/msun/src/s_nan.c
void bzero(void *b, size_t len);

// for isxdigit
#include </usr/include/sys/ctype.h>

static int
digittoint(int c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    c |= 0x20;
    c -= 'a';

    if ((unsigned int)c <= 5) {
        return c + 10;
    } else {
        return 0;
    }
}

#endif /* COLIBC_STRINGS_H__ */
