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

__attribute__((format(printf, 1, 2))) static void comment(char *fmt, ...);
static void comment(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  printf("# ");
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

void push(Reg src) {
  printf("\tpush %s\n", reg64[src]);
  stack_size += 8;
}

void pushi(int src) {
  printf("\tpush %d\n", src);
  stack_size += 8;
}

void pop(Reg dst) {
  printf("\tpop %s\n", reg64[dst]);
  stack_size -= 8;
}

void emit_ret() {
  puts("\tret");
}

/* void load(Reg dst, size_t size, size_t offset) { */
/*   printf("\tmov %s, [rbp-%zu]\n", reg(dst, size), offset); */
/* } */

void load(Reg dst, size_t size) {
  emit("mov %s, [rax]", reg(dst, size));
}

void store(Reg src, size_t size) {
  emit("mov [rax], %s", reg(src, size));
}

/* void store(Reg src, size_t size, size_t offset) { */
/*   printf("\tmov [rbp-%zu], %s\n", offset, reg(src, size)); */
/* } */

void emit_lval(Node* node) {
  assert(node->tag == NVAR);

  emit("mov rax, rbp");
  emit("sub rax, %zu", node->offset);
  push(AX);
}

void compile(Node* node) {
  switch (node->tag) {
  case NVAR: {
    comment("start NVAR");
    emit_lval(node);
    pop(AX);
    load(AX, size_of(type_of(node)));
    push(AX);
    comment("end NVAR");
    break;
  }
  case NDEFVAR: {
    comment("start NDEFVAR");
    comment("end NDEFVAR");
    break;
  }
  case NASSIGN: {
    comment("start NASSIGN");
    emit_lval(node->lhs);
    compile(node->rhs);
    pop(DI);
    pop(AX);
    store(DI, size_of(type_of(node->lhs)));
    push(DI);
    comment("end NASSIGN");
    break;
  }
  case NPLUS:
    comment("start NPLUS");
    compile(node->lhs);
    compile(node->rhs);
    pop(DI);
    pop(AX);
    emit_add32(AX, DI);
    push(AX);
    comment("end NPLUS");
    break;
  case NMINUS:
    comment("start NMINUS");
    compile(node->lhs);
    compile(node->rhs);
    pop(DI);
    pop(AX);
    emit_sub32(AX, DI);
    push(AX);
    comment("end NMINUS");
    break;
  case NMUL:
    comment("start NMUL");
    compile(node->lhs);
    compile(node->rhs);
    pop(DI);
    pop(AX);
    emit_imul32(DI);
    push(AX);
    comment("end NMUL");
    break;
  case NDIV:
    comment("start NDIV");
    compile(node->lhs);
    compile(node->rhs);
    pop(DI);
    pop(AX);
    emit_movi(DX, 0);
    emit_div32(DI);
    push(AX);
    comment("end NDIV");
    break;
  case NEQ:
    comment("start NEQ");
    compile(node->lhs);
    compile(node->rhs);
    pop(DI);
    pop(AX);
    emit("cmp %s, %s", reg64[AX], reg64[DI]);
    emit("sete al");
    emit("movzb rax, al");
    push(AX);
    comment("end NEQ");
    break;
  case NNE:
    comment("start NNE");
    compile(node->lhs);
    compile(node->rhs);
    pop(DI);
    pop(AX);
    emit("cmp %s, %s", reg64[AX], reg64[DI]);
    emit("setne al");
    emit("movzb rax, al");
    push(AX);
    comment("end NNE");
    break;
  case NGE:
    comment("start NGE");
    compile(node->lhs);
    compile(node->rhs);
    pop(DI);
    pop(AX);
    emit("cmp %s, %s", reg64[AX], reg64[DI]);
    emit("setge al");
    emit("movzb rax, al");
    push(AX);
    comment("end NGE");
    break;
  case NGT:
    comment("start NGT");
    compile(node->lhs);
    compile(node->rhs);
    pop(DI);
    pop(AX);
    emit("cmp %s, %s", reg64[AX], reg64[DI]);
    emit("setg al");
    emit("movzb rax, al");
    push(AX);
    comment("end NGT");
    break;
  case NLE:
    comment("start NLE");
    compile(node->lhs);
    compile(node->rhs);
    pop(DI);
    pop(AX);
    emit("cmp %s, %s", reg64[AX], reg64[DI]);
    emit("setle al");
    emit("movzb rax, al");
    push(AX);
    comment("end NLE");
    break;
  case NLT:
    comment("start NLT");
    compile(node->lhs);
    compile(node->rhs);
    pop(DI);
    pop(AX);
    emit("cmp %s, %s", reg64[AX], reg64[DI]);
    emit("setl al");
    emit("movzb rax, al");
    push(AX);
    comment("end NLT");
    break;
  case NINT:
    comment("start NINT");
    pushi(node->integer);
    comment("end NINT");
    break;
  case NCALL: {
    comment("start NCALL");
    // function call
    for (size_t i = 0; i < node->args->length; i++) {
      compile(node->args->ptr[i]);
    }
    for (ptrdiff_t i = node->args->length - 1; i >= 0; i--) {
      pop(argregs[i]);
    }

    push(R10);
    push(R11);
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
    pop(R11);
    pop(R10);

    if (ispadding) {
      emit("add rsp, 8");
      stack_size -= 8;
    }

    push(AX);
    comment("end NCALL");
    break;
  }
  case NEXPR_STMT:
    comment("start NEXPR_STMT");
    compile(node->expr);
    comment("end NRETURN");
    break;
  case NRETURN:
    comment("start NRETURN");
    compile(node->expr);
    pop(AX);
    emit_jmp(func_end_label);
    comment("end NRETURN");
    break;
  case NIF: {
    comment("start NIF");
    compile(node->cond);
    pop(AX);
    emit_cmpi(AX, 0);
    char* l = new_label("end");
    emit_je(l);
    compile(node->then);
    printf("%s:\n", l);
    comment("end NIF");
    break;
  }
  case NIFELSE: {
    comment("start NIFELSE");
    compile(node->cond);
    pop(AX);
    emit_cmpi(AX, 0);
    char* els = new_label("else");
    emit_je(els);
    compile(node->then);
    char* end = new_label("end");
    emit_jmp(end);
    printf("%s:\n", els);
    compile(node->els);
    printf("%s:\n", end);
    comment("end NIFELSE");
    break;
  }
  case NWHILE: {
    comment("start NWHILE");
    char* begin = new_label("begin");
    char* end = new_label("end");
    printf("%s:\n", begin);
    compile(node->cond);
    pop(AX);
    emit_cmpi(AX, 0);
    emit_je(end);
    compile(node->body);
    emit_jmp(begin);
    printf("%s:\n", end);
    comment("end NWHILE");
    break;
  }
  case NFOR: {
    comment("start NFOR");
    char* begin = new_label("begin");
    char* end = new_label("end");
    compile(node->init);
    printf("%s:\n", begin);
    compile(node->cond);
    pop(AX);
    emit_cmpi(AX,0);
    emit_je(end);
    compile(node->body);
    compile(node->step);
    emit_jmp(begin);
    printf("%s:\n", end);
    comment("end NFOR");
    break;
  }
  case NBLOCK: {
    comment("start NBLOCK");
    for (size_t i = 0; i < node->stmts->length; i++) {
      compile(node->stmts->ptr[i]);
    }
    comment("end NBLOCK");
    break;
  }
  case NFUNCDEF: {
    comment("start NFUNCDEF");
    func_end_label = new_label("end");

    puts(".text");
    printf(".global %s\n", node->name);
    printf("%s:\n", node->name);

    emit_enter(node->local_size);

    for (size_t i = 0; i < node->params->length; i++) {
      Node* param = node->params->ptr[i];
      emit_lval(param);
      store(argregs[i], size_of(type_of(param)));
    }
    compile(node->body);

    printf("%s:\n", func_end_label);
    emit_leave();
    emit_ret();
    comment("end NFUNCDEF");
    break;
  }
  default:
    eprintf("emit error: ");
    dump_node(node, 0);
    error(" unimplemented\n");
  }
}
