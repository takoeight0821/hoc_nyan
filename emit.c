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

void emit_sub(char* dst, char* src) {
  printf("\tsub %s, %s\n", dst, src);
}

void emit_imul(char* dst, char* src) {
  printf("\timul %s, %s\n", dst, src);
}

void emit_push(char* src) {
  printf("\tpush %s\n", src);
}

void emit_pop(char* dst) {
  printf("\tpop %s\n", dst);
}

void compile(Node* node) {
  switch (node->tag) {
  case NPLUS:
    compile(node->lhs);
    emit_push("rax");
    compile(node->rhs);
    emit_pop("rcx");
    emit_add("ecx", "eax");
    emit_mov("rax", "rcx");
    break;
  case NMINUS:
    compile(node->lhs);
    emit_push("rax");
    compile(node->rhs);
    emit_pop("rcx");
    emit_sub("ecx", "eax");
    emit_mov("rax", "rcx");
    break;
  case NMUL:
    compile(node->lhs);
    emit_push("rax");
    compile(node->rhs);
    emit_pop("rcx");
    emit_imul("ecx", "eax");
    emit_mov("rax", "rcx");
    break;
  case NINT:
    emit_mov("eax", format("%d", node->integer));
    break;
  case NERROR:
    error("compile NERROR");
    break;
  }
}
