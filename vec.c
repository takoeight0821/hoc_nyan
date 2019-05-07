#include "hoc.h"

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
  vec_extend(v, v->length + 1);
  v->length++;
  v->ptr[v->length] = elem;
}

void* vec_pop(Vector* v) {
  v->length--;
  return v->ptr[v->length];
}
