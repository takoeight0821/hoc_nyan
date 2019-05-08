#include "hoc.h"

char *format(const char *fmt, ...) {
  char buf[2048];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  return strdup(buf);
}

void error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, fmt, ap);
  va_end(ap);
  exit(1);
}

Vector* new_vec() {
  Vector* v = malloc(sizeof(Vector));
  v->ptr = malloc(sizeof(void*) * 16);
  v->capacity = 16;
  v->length = 0;
  return v;
}

void vec_extend(Vector* v, size_t new_capacity) {
  if (new_capacity <= v->capacity)
    return;

  v->capacity = new_capacity;
  v->ptr = realloc(v->ptr, sizeof(void*) * new_capacity);
}

void* vec_get(Vector* v, size_t i) {
  return v->ptr[i];
}

void vec_set(Vector* v, size_t i, void* elem) {
  vec_extend(v, i);

  v->ptr[i] = elem;
}

void vec_push(Vector* v, void* elem) {
  v->ptr[v->length] = elem;
  vec_extend(v, v->length + 1);
  v->length++;
}

void* vec_pop(Vector* v) {
  v->length--;
  return v->ptr[v->length];
}
