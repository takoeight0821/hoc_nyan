typedef long size_t;
typedef struct Vector {
  void **ptr;
  size_t capacity;
  size_t length;
} Vector;
void* calloc();
void* realloc();
