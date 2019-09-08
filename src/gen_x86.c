#include "hoc.h"

static char* regs[NUM_REGS] = { "r10", "r11", "rbx", "r12", "r13", "r14", "r15" };
static char* regs32[NUM_REGS] = { "r10d", "r11d", "ebx", "r12d", "r13d", "r14d", "r15d" };
static char* regs8[NUM_REGS] = { "r10b", "r11b", "bl", "r12b", "r13b", "r14b", "r15b" };

static char* func_end_label;
static int label_id = 0;

static char* new_label(char* name) {
  return format(".L%s%u", name, label_id++);
}

static char* get_reg(int rn, size_t size) {
  switch (size) {
  case 1: return regs8[rn];
  case 4: return regs32[rn];
  case 8: return regs[rn];
  default: error("invalid size register\n");
  }
}

#ifndef __hoc__
__attribute__((format(printf, 1, 2))) static void emit(char *fmt, ...);
#endif

static void emit(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  printf("\t");
  vprintf(fmt, ap);
  printf("\n");
}

static void emit_const(Type* type, Node* node) {
  if (!node) {
    return;
  }

  switch (node->tag) {
  case NINT: {
    if (size_of(type) == 1) {
      emit("db %d", node->integer);
    } else if (size_of(type) == 4) {
      emit("dd %d", node->integer);
    } else if (size_of(type) == 8) {
      emit("dq %d", node->integer);
    } else {
      bad_token(node->token, "emit error: emit_const(invalid size)");
    }
    break;
  }
  case NGVAR: {
    if (size_of(type) == 1) {
      emit(".byte %s", node->name);
    } else if (size_of(type) == 4) {
      emit(".int %s", node->name);
    } else if (size_of(type) == 8) {
      emit(".quad %s", node->name);
    } else {
      bad_token(node->token, "emit error: emit_const(invalid size)");
    }
    break;
  }
  default:
    bad_token(node->token, "emit error: value is not constant");
  }
}

static void emit_mov(char* dst, char* src) {
  if (!streq(dst, src)) {
    emit("mov %s, %s", dst, src);
  }
}

static void emit_ir(IR* ir) {
  switch (ir->op) {
  case IIMM: {
    emit("mov %s, %d", get_reg(ir->r0->real_reg, ir->r0->size), ir->imm_int);
    break;
  }
  case ILABEL: {
    emit("lea %s, [%s]", get_reg(ir->r0->real_reg, ir->r0->size), ir->label);
    break;
  }
  case IADD: {
    emit_mov(get_reg(ir->r0->real_reg, ir->r0->size), get_reg(ir->r1->real_reg, ir->r0->size));
    emit("add %s, %s", get_reg(ir->r0->real_reg, ir->r0->size), get_reg(ir->r2->real_reg, ir->r0->size));
    break;
  }
  case ISUB: {
    emit_mov(get_reg(ir->r0->real_reg, ir->r0->size), get_reg(ir->r1->real_reg, ir->r0->size));
    emit("sub %s, %s", get_reg(ir->r0->real_reg, ir->r0->size), get_reg(ir->r2->real_reg, ir->r0->size));
    break;
  }
  case IMUL: {
    emit_mov("rax", get_reg(ir->r1->real_reg, 8));
    emit("imul %s", get_reg(ir->r2->real_reg, ir->r2->size));
    emit_mov(get_reg(ir->r0->real_reg, 8), "rax");
    break;
  }
  case IDIV: {
    emit_mov("rdx", "0");
    emit_mov("rax", get_reg(ir->r1->real_reg, 8));
    emit("div %s", get_reg(ir->r2->real_reg, ir->r2->size));
    emit_mov(get_reg(ir->r0->real_reg, 8), "rax");
    break;
  }
  case IRET: {
    emit("mov rax, %s", get_reg(ir->r1->real_reg, 8));
    emit("jmp %s", func_end_label);
    break;
  }
  }
}

static void emit_block(Block* block) {
  printf("%s:\n", block->label);
  for (size_t i = 0; i < block->instrs->length; i++) {
    emit_ir(block->instrs->ptr[i]);
  }
}

static void emit_function(IFunc* func) {
  func_end_label = new_label("end");

  if (!func->is_static) {
    printf("global %s\n", func->name);
  }
  printf("%s:\n", func->name);

  emit("push rbp");
  emit("mov rbp, rsp");
  emit("sub rsp, %d", count_stack_size(func));
  emit("and rsp, -16");

  for (size_t i = 0; i < func->blocks->length; i++) {
    emit_block(func->blocks->ptr[i]);
  }

  printf("%s:\n", func_end_label);
  emit("leave");
  emit("ret");
}

void gen_x86(IProgram* prog) {
  puts("section .data");
  for (GVar* gvar = prog->globals; gvar != NULL; gvar = gvar->next) {
    if (gvar->init && !gvar->is_extern) {
      printf("%s:\n", gvar->name);
      emit_const(gvar->type, gvar->init);
    } else if (gvar->inits && !gvar->is_extern) {
      printf("%s:\n", gvar->name);
      for (size_t i = 0; i < gvar->inits->length; i++) {
        emit_const(gvar->type->ptr_to, gvar->inits->ptr[i]);
      }
    }
  }

  puts("section .bss");
  for (GVar* gvar = prog->globals; gvar != NULL; gvar = gvar->next) {
    if (gvar->init == NULL && gvar->inits == NULL && !gvar->is_extern) {
      printf("%s:\n", gvar->name);
      emit("resb %zu", size_of(gvar->type));
    }
  }

  puts("section .text");
  for (size_t i = 0; i < prog->ifuncs->length; i++) {
    emit_function(prog->ifuncs->ptr[i]);
  }
}