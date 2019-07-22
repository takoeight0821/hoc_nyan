#include "hoc.h"

char* format(const char *fmt, ...) {
  char* buf = calloc(2048, sizeof(char));
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

StringBuilder* new_sb(void) {
  StringBuilder* sb = calloc(1, sizeof(StringBuilder));
  sb->buf = calloc(8, sizeof(char));
  sb->capacity = 8;
  sb->length = 0;
  return sb;
}

void sb_putc(StringBuilder* sb, char c) {
  while (sb->length >= sb->capacity) {
    sb->capacity += 8;
    sb->buf = realloc(sb->buf, sizeof(char) * sb->capacity);
  }
  sb->buf[sb->length] = c;
  sb->length++;
}

void sb_puts(StringBuilder* sb, char* str) {
  for (size_t i = 0; i < strlen(str); i++) {
    sb_putc(sb, str[i]);
  }
}

char* sb_run(StringBuilder* sb) {
  sb_putc(sb, '\0');
  char* ret = sb->buf;
  sb_destory(sb);
  return ret;
}

void sb_destory(StringBuilder* sb) {
  free(sb);
}
