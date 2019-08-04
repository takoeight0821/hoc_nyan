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

void runtest() {
  test_vector();
  printf("OK\n");
}

int main(int argc, char** argv)
{
  bool dump = false;
  if (argc < 1) {
    error("./hoc file_name\n");
  }

  if (streq(argv[1], "-test")) {
    runtest();
    return 0;
  }

  if (streq(argv[1], "-dump")) {
    dump = true;
  }

  Token* tokens = lex(argv[dump ? 2 : 1]);

  if (dump) {
    for (Token* t = tokens; t != NULL; t = t->next) {
      dump_token(t);
    }
    eprintf("\n");
  }

  /* Program* prog = parse(tokens); */
  /* sema(prog); */

  /* if (dump) { */
  /*   for (size_t i = 0; i < prog->funcs->length; i++) { */
  /*     dump_function(prog->funcs->ptr[i]); */
  /*   } */
  /* } */

  /* gen_x86(prog); */

  return 0;
}
