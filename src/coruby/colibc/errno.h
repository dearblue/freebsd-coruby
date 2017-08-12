#ifndef COLIBC_ERRNO_H
#define COLIBC_ERRNO_H 1

#include <sys/errno.h>
#include "colibc.h"

COLIBC_API int colibc_exittrap(void **status, void *(*func)(void *), void *user);
COLIBC_API int *colibc_errno(void);

COLIBC_API _Noreturn void abort(void);
COLIBC_API _Noreturn void exit(int);
#define errno (*colibc_errno())
//_Thread_local int errno;

#endif /* COLIBC_ERRNO_H */
