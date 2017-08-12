#ifndef COLIBC_STRING_H
#define COLIBC_STRING_H 1

#include <sys/ctype.h>
#include <sys/types.h>
/*
 * NOTE: math.h と sys/systm.h でそれぞれ定義される log() が衝突するため
 * NOTE: sys/systm.h の log() を systm_h__log() として宣言させる
 */
#define log systm_h__log
#include <sys/systm.h>
#undef log
#include <sys/malloc.h>

#define free(p) free(p, M_CORUBY_MRB)
#define realloc(p, size) realloc(p, size, M_CORUBY_MRB, M_WAITOK)
#define malloc(size) malloc(size, M_CORUBY_MRB, M_WAITOK)

MALLOC_DECLARE(M_CORUBY_MRB);

/*
 * sys/param.h で定義されているマクロ関数を削除。
 * これらは mruby/src/fmt_fp.c にて定義されているため。
 */
#undef MIN
#undef MAX

#endif /* COLIBC_STRING_H */
