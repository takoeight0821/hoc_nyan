#include <stdio.h>
#include "hoc.h"

void expect(int line, int expected, int actual) {
  if (expected == actual)
    return;
  error("%d: %d expected, but got %d\n", line, expected, actual);
  exit(1);
}

void test_vector() {
  Vector *vec = new_vec();
  expect(__LINE__, 0, vec->length);

  for (int i = 0; i < 100; i++)
    vec_pushi(vec, i);

  expect(__LINE__, 100, vec->length);
  expect(__LINE__, 0, (intptr_t)vec->ptr[0]);
  expect(__LINE__, 50, (intptr_t)vec->ptr[50]);
  expect(__LINE__, 99, (intptr_t)vec->ptr[99]);
}

void test_map() {
  Map *map = new_map();
  expect(__LINE__, 0, (intptr_t)map_get(map, "foo"));

  map_put(map, "foo", (void *)2);
  expect(__LINE__, 2, (intptr_t)map_get(map, "foo"));

  map_put(map, "bar", (void *)4);
  expect(__LINE__, 4, (intptr_t)map_get(map, "bar"));

  map_put(map, "foo", (void *)6);
  expect(__LINE__, 6, (intptr_t)map_get(map, "foo"));
}

void runtest() {
  test_vector();
  test_map();
  printf("OK\n");
}

int main(int argc, char** argv)
{
  if (streq(argv[1], "-test")) {
    runtest();
    return 0;
  }

  FILE* fp;
  if ((fp = fopen(argv[1], "r")) == NULL) {
    return -1;
  }

  Vector* tokens = lex(fp);

  for (size_t i = 0; i < tokens->length; i++) {
    dump_token(tokens->ptr[i]);
  }
  eprintf("\n");

  fclose(fp);

  Program* prog = parse(tokens);

  sema(prog);

  for (size_t i = 0; i < prog->funcs->length; i++) {
    dump_function(prog->funcs->ptr[i]);
  }

  gen_x86(prog);

  return 0;
}
