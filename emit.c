#include "hoc.h"

static char* reg64[] = { "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11" };
static char* reg32[] = { "eax", "edi", "esi", "edx", "ecx", "r8d", "r9d", "r10d", "r11d" };
static Reg argregs[] = {DI, SI, DX, CX, R8, R9};
static unsigned int label_id = 0;
static int stack_size = 0;
static char* func_end_label;

static char* reg(Reg r, size_t s) {
  if (s == 4) {
    return reg32[r];
  }
  assert(s == 8);
  return reg64[r];
}

static char* new_label(char* name) {
  return format(".L%s%u", name, label_id++);
}

__attribute__((format(printf, 1, 2))) static void emit(char *fmt, ...);

static void emit(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  printf("\t");
  vprintf(fmt, ap);
  printf("\n");
}

void emit_enter(int size) {
  emit("push rbp");
  emit("mov rbp, rsp");
  emit("sub rsp, %d", size);
  stack_size += size;
}

void emit_leave() {
  emit("leave");
}

void emit_je(char* label) {
  printf("\tje %s\n", label);
}

void emit_jmp(char* label) {
  printf("\tjmp %s\n", label);
}

void emit_mov(Reg dst, Reg src) {
  printf("\tmov %s, %s\n", reg64[dst], reg64[src]);
}

void emit_movi(Reg dst, long src) {
  printf("\tmov %s, %ld\n", reg64[dst], src);
}

void emit_cmp(Reg reg1, Reg reg2) {
  printf("\tcmp %s, %s\n", reg64[reg1], reg64[reg2]);
}

void emit_cmpi(Reg reg, int i) {
  printf("\tcmp %s, %d\n", reg64[reg], i);
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
  stack_size += 8;
}

void emit_pushi(int src) {
  printf("\tpush %d\n", src);
  stack_size += 8;
}

void emit_pop(Reg dst) {
  printf("\tpop %s\n", reg64[dst]);
  stack_size -= 8;
}

void emit_ret() {
  puts("\tret");
}

void load(Reg dst, size_t size, size_t offset) {
  printf("\tmov %s, [rbp-%zu]\n", reg(dst, size), offset);
}

void store(Reg src, size_t size, size_t offset) {
  printf("\tmov [rbp-%zu], %s\n", offset, reg(src, size));
}

void compile(Node* node) {
  switch (node->tag) {
  case NVAR: {
    if (node->offset == 0) {
      error("%s is not defined\n", node->name);
    }
    load(AX, size_of(type_of(node)), node->offset);
    emit_push(AX);
    break;
  }
  case NDEFVAR: {
    break;
  }
  case NASSIGN: {
    compile(node->rhs);
    emit_pop(AX);
    store(AX, size_of(type_of(node->lhs)), node->lhs->offset);
    emit_push(AX);
    break;
  }
  case NPLUS:
    compile(node->lhs);
    compile(node->rhs);
    emit_pop(DI);
    emit_pop(AX);
    emit_add32(AX, DI);
    emit_push(AX);
    break;
  case NMINUS:
    compile(node->lhs);
    compile(node->rhs);
    emit_pop(DI);
    emit_pop(AX);
    emit_sub32(AX, DI);
    emit_push(AX);
    break;
  case NMUL:
    compile(node->lhs);
    compile(node->rhs);
    emit_pop(DI);
    emit_pop(AX);
    emit_imul32(DI);
    emit_push(AX);
    break;
  case NDIV:
    compile(node->lhs);
    compile(node->rhs);
    emit_pop(DI);
    emit_pop(AX);
    emit_movi(DX, 0);
    emit_div32(DI);
    emit_push(AX);
    break;
  case NEQ:
    compile(node->lhs);
    compile(node->rhs);
    emit_pop(DI);
    emit_pop(AX);
    printf("\tcmp %s, %s\n", reg64[AX], reg64[DI]);
    printf("\tsete al\n");
    printf("\tmovzb eax, al\n");
    emit_push(AX);
    break;
  case NNE:
    compile(node->lhs);
    compile(node->rhs);
    emit_pop(DI);
    emit_pop(AX);
    printf("\tcmp %s, %s\n", reg64[AX], reg64[DI]);
    printf("\tsetne al\n");
    printf("\tmovzb eax, al\n");
    emit_push(AX);
    break;
  case NGE:
    compile(node->lhs);
    compile(node->rhs);
    emit_pop(AX);
    emit_pop(DI);
    printf("\tcmp %s, %s\n", reg64[AX], reg64[DI]);
    printf("\tsetle al\n");
    printf("\tmovzb eax, al\n");
    emit_push(AX);
    break;
  case NGT:
    compile(node->lhs);
    compile(node->rhs);
    emit_pop(AX);
    emit_pop(DI);
    printf("\tcmp %s, %s\n", reg64[AX], reg64[DI]);
    printf("\tsetl al\n");
    printf("\tmovzb eax, al\n");
    emit_push(AX);
    break;
  case NLE:
    compile(node->lhs);
    compile(node->rhs);
    emit_pop(DI);
    emit_pop(AX);
    printf("\tcmp %s, %s\n", reg64[AX], reg64[DI]);
    printf("\tsetle al\n");
    printf("\tmovzb eax, al\n");
    emit_push(AX);
    break;
  case NLT:
    compile(node->lhs);
    compile(node->rhs);
    emit_pop(DI);
    emit_pop(AX);
    printf("\tcmp %s, %s\n", reg64[AX], reg64[DI]);
    printf("\tsetl al\n");
    printf("\tmovzb eax, al\n");
    emit_push(AX);
    break;
  case NINT:
    emit_pushi(node->integer);
    break;
  case NCALL: {
    // function call
    for (size_t i = 0; i < node->args->length; i++) {
      compile(node->args->ptr[i]);
    }
    for (ptrdiff_t i = node->args->length - 1; i >= 0; i--) {
      emit_pop(argregs[i]);
    }

    emit_push(R10);
    emit_push(R11);
    emit_movi(AX, 0);

    // スタックをがりがりいじりながらコード生成してるので、
    // rspのアライメントをうまいこと扱う必要がある。
    // push/popの回数をカウントしておく。偶数なら何もしない。奇数ならrsp -= 8。関数が戻ってきたらrsp += 8
    bool ispadding = stack_size % 16;
    if (ispadding) {
      emit("sub rsp, 8");
      stack_size += 8;
    }
    emit("call %s", node->name);
    emit_pop(R11);
    emit_pop(R10);

    if (ispadding) {
      emit("add rsp, 8");
      stack_size -= 8;
    }

    emit_push(AX);
    break;
  }
  case NRETURN:
    compile(node->ret);
    emit_pop(AX);
    emit_jmp(func_end_label);
    break;
  case NIF: {
    compile(node->cond);
    emit_pop(AX);
    emit_cmpi(AX, 0);
    char* l = new_label("end");
    emit_je(l);
    compile(node->then);
    printf("%s:\n", l);
    break;
  }
  case NIFELSE: {
    compile(node->cond);
    emit_pop(AX);
    emit_cmpi(AX, 0);
    char* els = new_label("else");
    emit_je(els);
    compile(node->then);
    char* end = new_label("end");
    emit_jmp(end);
    printf("%s:\n", els);
    compile(node->els);
    printf("%s:\n", end);
    break;
  }
  case NWHILE: {
    char* begin = new_label("begin");
    char* end = new_label("end");
    printf("%s:\n", begin);
    compile(node->cond);
    emit_pop(AX);
    emit_cmpi(AX, 0);
    emit_je(end);
    compile(node->body);
    emit_jmp(begin);
    printf("%s:\n", end);
    break;
  }
  case NFOR: {
    char* begin = new_label("begin");
    char* end = new_label("end");
    compile(node->init);
    printf("%s:\n", begin);
    compile(node->cond);
    emit_pop(AX);
    emit_cmpi(AX,0);
    emit_je(end);
    compile(node->body);
    compile(node->step);
    emit_jmp(begin);
    printf("%s:\n", end);
    break;
  }
  case NBLOCK: {
    for (size_t i = 0; i < node->stmts->length; i++) {
      compile(node->stmts->ptr[i]);
    }
    break;
  }
  case NFUNCDEF: {
    func_end_label = new_label("end");

    puts(".text");
    printf(".global %s\n", node->name);
    printf("%s:\n", node->name);

    emit_enter(node->local_size);

    for (size_t i = 0; i < node->params->length; i++) {
      Node* param = node->params->ptr[i];
      store(argregs[i], size_of(type_of(param)), param->offset);
    }
    compile(node->body);

    printf("%s:\n", func_end_label);
    emit_leave();
    emit_ret();
    break;
  }
  default:
    eprintf("emit error: ");
    dump_node(node, 0);
    error(" unimplemented\n");
  }
}
