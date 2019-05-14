#include "hoc.h"

char* reg64[] = { "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11" };
char* reg32[] = { "eax", "edi", "esi", "edx", "ecx", "r8d", "r9d", "r10d", "r11d" };

void emit_enter(int size, int nest) {
  printf("\tpush rbp\n");
  printf("\tmov rbp, rsp\n");
  printf("\tsub rsp, %d\n", size);
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

void emit_div32(Reg src) {
  printf("\tdiv %s\n", reg32[src]);
}

void emit_push(Reg src) {
  printf("\tpush %s\n", reg64[src]);
}

void emit_pushi(int src) {
  printf("\tpush %d\n", src);
}

void emit_pop(Reg dst) {
  printf("\tpop %s\n", reg64[dst]);
}

void emit_ret() {
  puts("\tret");
}

void emit_load32(Reg dst, int offset) {
  printf("\tmov %s, [rbp-%d]\n", reg32[dst], offset);
}

void emit_store32(Reg dst, int offset) {
  printf("\tmov [rbp-%d], %s\n", offset, reg32[dst]);
}

void compile(Node* node, Map* vars) {
  switch (node->tag) {
  case NVAR: {
    int offset = (int)map_get(vars, node->ident);
    if (offset == 0) {
      error("%s is not defined\n", node->ident);
    }
    emit_load32(AX, offset);
    emit_push(AX);
    break;
  }
  case NASSIGN: {
    int offset = (int)map_get(vars, node->lhs->ident);
    compile(node->rhs, vars);
    emit_pop(AX);
    emit_store32(AX, offset);
    emit_push(AX);
    break;
  }
  case NPLUS:
    compile(node->lhs, vars);
    compile(node->rhs, vars);
    emit_pop(DI);
    emit_pop(AX);
    emit_add32(AX, DI);
    emit_push(AX);
    break;
  case NMINUS:
    compile(node->lhs, vars);
    compile(node->rhs, vars);
    emit_pop(DI);
    emit_pop(AX);
    emit_sub32(AX, DI);
    emit_push(AX);
    break;
  case NMUL:
    compile(node->lhs, vars);
    compile(node->rhs, vars);
    emit_pop(DI);
    emit_pop(AX);
    emit_imul32(DI);
    emit_push(AX);
    break;
  case NDIV:
    compile(node->lhs, vars);
    compile(node->rhs, vars);
    emit_pop(DI);
    emit_pop(AX);
    emit_movi(DX, 0);
    emit_div32(DI);
    emit_push(AX);
    break;
  case NINT:
    emit_pushi(node->integer);
    break;
  case NRETURN:
    compile(node->ret, vars);
    emit_pop(AX);
    emit_leave();
    emit_ret();
    break;
  case NSTMTS: {
    for (size_t i = 0; i < node->stmts->length; i++) {
      compile(node->stmts->ptr[i], vars);
    }
    break;
  }
  }
}
