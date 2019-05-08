#include <stdio.h>
#include "hoc.h"

int main(int argc, char** argv)
{
  FILE* fp = fopen(argv[1], "r");

  Vector* tokens = lex(fp);

  fclose(fp);

  Node* ast = parse(tokens);

  puts(".intel_syntax noprefix");
  puts(".text");
  puts(".global main");
  puts("main:");
  emit_mov("rax", "0");
  emit_add("rax", format("%d", ast->lhs->integer));
  emit_add("rax", format("%d", ast->rhs->integer));
  puts("\tret");

  return 0;
}
