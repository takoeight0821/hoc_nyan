#include <stdio.h>
#include "hoc.h"

void emit_mov(const char * dst, const char * src)
{
  printf("\tmov %s, %s\n", dst, src);
}

void emit_add(const char * dst, const char * src)
{
  printf("\tadd %s, %s\n", dst, src);
}

void emit_add_const(const char * dst, int i)
{
  char buf[256];
  sprintf(buf, "%d", i);
  printf("\tadd %s, %s\n", dst, buf);
}

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
  emit_add_const("rax", ast->lhs->integer);
  emit_add_const("rax", ast->rhs->integer);
  puts("\tret");

  return 0;
}
