#include <stdio.h>
#include "hoc.h"

int main(int argc, char** argv)
{
  FILE* fp = fopen(argv[1], "r");

  Vector* tokens = lex(fp);

  /* for (int i = 0; i < tokens->length; i++) { */
  /*   dump_token(*(Token*)vec_get(tokens, i)); */
  /* } */

  fclose(fp);

  Node* ast = parse(tokens);

  /* dump_node(ast, 0); */

  puts(".intel_syntax noprefix");
  puts(".text");
  puts(".global main");
  puts("main:");
  compile(ast);
  /* emit_mov("rax", "0"); */
  /* emit_add("rax", format("%d", ast->lhs->integer)); */
  /* emit_add("rax", format("%d", ast->rhs->integer)); */
  puts("\tret");

  return 0;
}
