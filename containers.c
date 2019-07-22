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

Map *new_map() {
  Map *map = calloc(1, sizeof(Map));
  map->keys = new_vec();
  map->vals = new_vec();
  return map;
}

void map_put(Map *map, char *key, void *val) {
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void map_puti(Map *map, char *key, intptr_t val) {
  map_put(map, key, (void*)val);
}

void *map_get(Map *map, char *key) {
  for (int i = map->keys->length - 1; i >= 0; i--) {
    if (streq(map->keys->ptr[i], key)) {
      return map->vals->ptr[i];
    }
  }
  return NULL;
}

intptr_t map_geti(Map *map, char *key) {
  return (intptr_t)map_get(map, key);
}

int map_has_key(Map* map, char *key) {
  for (int i = map->keys->length - 1; i >= 0; i--) {
    if (streq(map->keys->ptr[i], key))
      return 1;
  }
  return 0;
}
