#ifndef COLIBC_SETJMP_H
#define COLIBC_SETJMP_H 1

#include <machine/setjmp.h>
/*
 * NOTE: math.h と sys/systm.h でそれぞれ定義される log() が衝突するため
 * NOTE: sys/systm.h の log() を systm_h__log() として宣言させる
 */
#define log systm_h__log
#include <sys/systm.h>
#undef log

#define _longjmp longjmp
#define _setjmp setjmp

#endif /* COLIBC_SETJMP_H */
