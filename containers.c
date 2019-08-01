#include "hoc.h"

Vector* new_vec() {
  Vector* v = calloc(1, sizeof(Vector));
  v->ptr = calloc(16, sizeof(void*));
  v->capacity = 16;
  v->length = 0;
  return v;
}

void vec_push(Vector* v, void* elem) {
  if (v->capacity == v->length) {
    v->capacity *= 2;
    v->ptr = realloc(v->ptr, sizeof(void*) * v->capacity);
  }
  v->ptr[v->length++] = elem;
}

void vec_pushi(Vector* v, intptr_t elem) {
  vec_push(v, (void*)elem);
}

void* vec_pop(Vector* v) {
  v->length--;
  return v->ptr[v->length];
}

intptr_t vec_popi(Vector* v) {
  return (intptr_t)vec_pop(v);
}

void* vec_last(Vector* v) {
  return v->ptr[v->length - 1];
}
