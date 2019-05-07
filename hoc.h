#include <stdlib.h>

typedef struct {
  void **ptr;
  size_t capacity;
  size_t length;
} Vector;

// vec.c
void* vec_get(Vector* v, size_t i);
void vec_set(Vector* v, size_t i, void* elem);
void vec_push(Vector* v, void* elem);
void* vec_pop(Vector* v);
