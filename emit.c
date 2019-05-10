#include "hoc.h"

char* reg64[] = { "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11" };
char* reg32[] = { "eax", "edi", "esi", "edx", "ecx", "r8d", "r9d", "r10d", "r11d" };

void emit_enter(int size, int nest) {
  printf("\tenter %d, %d\n", size, nest);
}

void emit_leave() {
  puts("\tleave");
}

void emit_mov(Reg dst, Reg src) {
  printf("\tmov %s, %s\n", reg64[dst], reg64[src]);
}

void emit_movi(Reg dst, long src) {
  printf("\tmov %s, %ld\n", reg64[dst], src);
}

void emit_add32(Reg dst, Reg src) {
  printf("\tadd %s, %s\n", reg32[dst], reg32[src]);
}

void emit_sub32(Reg dst, Reg src) {
  printf("\tsub %s, %s\n", reg32[dst], reg32[src]);
}

void emit_imul32(Reg src) {
  printf("\timul %s\n", reg32[src]);
}

void emit_mul32(Reg src) {
  printf("\tmul %s\n", reg32[src]);
}

void emit_push(Reg src) {
  printf("\tpush %s\n", reg64[src]);
}

void emit_pop(Reg dst) {
  printf("\tpop %s\n", reg64[dst]);
}

void emit_ret() {
  puts("\tret");
}

void compile(Node* node) {
  switch (node->tag) {
  case NPLUS:
    compile(node->lhs);
    emit_push(AX);
    compile(node->rhs);
    emit_pop(DI);
    emit_add32(DI, AX);
    emit_mov(AX, DI);
    break;
  case NMINUS:
    compile(node->lhs);
    emit_push(AX);
    compile(node->rhs);
    emit_pop(DI);
    emit_sub32(DI, AX);
    emit_mov(AX, DI);
    break;
  case NMUL:
    compile(node->lhs);
    emit_push(AX);
    compile(node->rhs);
    emit_pop(DI);
    emit_imul32(DI);
    break;
  case NINT:
    emit_movi(AX, node->integer);
    break;
  case NRETURN:
    compile(node->ret);
    emit_leave();
    emit_ret();
    break;
  case NSTMTS: {
    for (size_t i = 0; i < node->stmts->length; i++) {
      compile(vec_get(node->stmts, i));
    }
    break;
  }
  }
}
