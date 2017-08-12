#ifndef COLIBC_STDLIB_H
#define COLIBC_STDLIB_H 1

#include <sys/types.h>
#include <sys/param.h>
#include "errno.h"

/* from /usr/include/stdlib.h */
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

double strtod(const char *, char **);

#endif /* COLIBC_STDLIB_H */
