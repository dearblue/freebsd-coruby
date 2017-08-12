#include "errno.h"
#include <machine/setjmp.h>
#include <sys/_null.h>
#include <sys/types.h>
#include <sys/systm.h>
#include "colibc.h"


/* time(3) */

#include <sys/time.h>

time_t
time(time_t *tloc)
{
    struct bintime bt;
    bintime(&bt);
    if (tloc) {
        *tloc = (time_t)bt.sec;
    }

    return (time_t)bt.sec;
}


thread_local jmp_buf env;

int
colibc_exittrap(void **status, void *(*func)(void *), void *user)
{
    int tag = setjmp(env);
    if (tag == 0) {
        *status = func(user);
    } else {
        *status = NULL;
    }

    return tag;
}

noreturn void
abort(void)
{
    longjmp(env, -1);
}

noreturn void
exit(int st)
{
    longjmp(env, st);
}

int *
colibc_errno(void)
{
    static thread_local int errno_ = 0;
    return &errno_;
}
