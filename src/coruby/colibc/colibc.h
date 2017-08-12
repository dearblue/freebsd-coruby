#ifndef COLIBC_H__
#define COLIBC_H__ 1

#ifndef thread_local
#   define thread_local _Thread_local
#endif

#ifndef noreturn
#   define noreturn _Noreturn
#endif

#define COLIBC_API __attribute__((visibility("hidden"))) extern

#endif /* COLIBC_H__ */
