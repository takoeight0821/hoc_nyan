#include "hoc.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

char *format(const char *fmt, ...) {
  char buf[2048];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  return strdup(buf);
}

void error(const char *fmt, ...) __attribute__((noreturn)) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, fmt, ap);
  va_end(ap);
  exit(1);
}
