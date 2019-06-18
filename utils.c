#include "hoc.h"

char* format(const char *fmt, ...) {
  char* buf = malloc(sizeof(char) * 2048);
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, 2048, fmt, ap);
  va_end(ap);
  return realloc(buf, sizeof(char) * (strlen(buf) + 1));
}

void error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  exit(1);
}

void eprintf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

int streq(char* s0, char* s1) {
  return strcmp(s0, s1) == 0;
}
