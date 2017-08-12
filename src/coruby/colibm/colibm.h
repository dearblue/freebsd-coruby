#ifndef COLIBM_H__
#define COLIBM_H__ 1

int isnan(double d);
int __isnanf(float f);
int __isnanl(long double e);

#define asm __asm
#include "math.h"
#include "fpmath.h"

#endif /* COLIBM_H__ */
