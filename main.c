#include <stdio.h>

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vec(void);
void vec_push(Vector* v, void* elem);
void *vec_pop(Vector *v);

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

  puts(".intel_syntax noprefix");
  puts(".text");
  puts(".global main");
  puts("main:");
  emit_mov("rax", "0");
  emit_add_const("rax", a);
  emit_add_const("rax", b);
  puts("\tret");

  return 0;
}
