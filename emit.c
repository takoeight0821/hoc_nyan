#include "hoc.h"

void emit_enter(int size, int nest) {
  printf("\tenter %d, %d\n", size, nest);
}

void emit_leave() {
  puts("\tleave");
}

void emit_mov(char* dst, char* src) {
  printf("\tmov %s, %s\n", dst, src);
}

void emit_add(char* dst, char* src) {
  printf("\tadd %s, %s\n", dst, src);
}

void compile(Node* node) {
  switch (node->tag) {
  case NPLUS:
    compile(node->lhs);
    emit_mov("r8", "rax");
    compile(node->rhs);
    emit_add("rax", "r8");
    break;
  case NINT:
    emit_mov("rax", format("%d", node->integer));
    break;
  case NERROR:
    error("compile NERROR");
    break;
  }
}
