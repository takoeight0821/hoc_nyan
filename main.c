#include <stdio.h>
#include "hoc.h"

int main()
{
  int a;
  int b;

  scanf("%d+%d", &a, &b);

  Node* ast = new_plus_node(new_int_node(a), new_int_node(b));

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
