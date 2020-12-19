/* https://gist.github.com/mori0091/45b275f61ac802fcabe7fb5dede7ca73 by mori0091 */

#include "tryCatch.h"

void raise(Ctx *ctx, const char *fmt, ...) {
  /* construct formatted string */
  va_list ap;
  va_start(ap, fmt);
  int len = vsnprintf(NULL, 0, fmt, ap);
  if (len < 0) {
    fprintf(stderr, "vsnprintf(NULL, 0, fmt, ...):%s\n", strerror(errno));
    exit(1);
  }
  char *buf = malloc(len + 1);
  if (vsnprintf(buf, len + 1, fmt, ap) < 0) {
    fprintf(stderr, "vsnprintf(buf, len, fmt, ...):%s\n", strerror(errno));
    exit(1);
  }
  /* throw as an exception */
  ctx->msg = buf;
  longjmp(ctx->e, -1);
}
