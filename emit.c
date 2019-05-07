#include "hoc.h"

void emit_mov(char* dst, char* src) {
  printf("\tmov %s, %s\n", dst, src);
}

void emit_add(char* dst, char* src) {
  printf("\tadd %s, %s\n", dst, src);
}
