#include <stdio.h>
#include "hoc.h"

int main(int argc, char** argv)
{
  FILE* fp;
  if ((fp = fopen(argv[1], "r")) == NULL) {
    return -1;
  }

  Vector* tokens = lex(fp);

  for (int i = 0; i < tokens->length; i++) {
    dump_token(*(Token*)(tokens->ptr[i]));
  }

  fclose(fp);

  Node* ast = parse(tokens);

  dump_node(ast, 0);

  puts(".intel_syntax noprefix");
  puts(".text");
  puts(".global main");
  puts("main:");
  emit_enter(0, 0);
  compile(ast);

  return 0;
}
