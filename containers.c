#include "hoc.h"

Vector* new_vec() {
  Vector* v = malloc(sizeof(Vector));
  v->ptr = malloc(sizeof(void*) * 16);
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

char* vec_to_string(Vector* v) {
  char* str = calloc(v->length + 1, sizeof(char));

  for (size_t i = 0; i < v->length; i++) {
    str[i] = (char)(intptr_t)(v->ptr[i]);
  }

  str[v->length] = '\0';

  return str;
}

Map *new_map() {
  Map *map = malloc(sizeof(Map));
  map->keys = new_vec();
  map->vals = new_vec();
  return map;
}

void map_put(Map *map, char *key, void *val) {
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void *map_get(Map *map, char *key) {
  // iは負になりうる
  // ptrdiff_tを使うべき？
  for (int i = map->keys->length - 1; i >= 0; i--) {
    if (streq(map->keys->ptr[i], key)) {
      return map->vals->ptr[i];
    }
  }
  return NULL;
}

int map_has_key(Map* map, char *key) {
  for (int i = map->keys->length - 1; i >= 0; i--) {
    if (streq(map->keys->ptr[i], key))
      return 1;
  }
  return 0;
}
