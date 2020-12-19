// https://gist.github.com/mori0091/45b275f61ac802fcabe7fb5dede7ca73

// to minimize use of "out vars"

#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TRYCATCH_H
#define TRYCATCH_H

typedef struct {
  char* msg;
  jmp_buf e;
} Ctx;

#define TRY(ctx) if (!setjmp((ctx)->e))

#define CATCH else

/*
 * raise(Ctx *ctx, const char *fmt, ...) - throw a formatted string as an
 * exception
 */
void raise(Ctx *ctx, const char *fmt, ...);

#endif
