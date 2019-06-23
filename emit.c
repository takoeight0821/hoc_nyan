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

void load(Reg dst, size_t size) {
  emit("mov %s, [rax]", reg(dst, size));
}

void store(Reg src, size_t size) {
  emit("mov [rax], %s", reg(src, size));
}

void emit_node(Node*);

void emit_lval(Node* node) {
  if (node->tag == NVAR) {
    emit("mov rax, rbp");
    emit("sub rax, %zu", node->var->offset);
    push(AX);
  } else {
    assert(node->tag == NDEREF);
    emit_node(node->expr);
  }
}

void emit_var(Var* var) {
  comment("start Var %s", var->name);
  emit("mov rax, rbp");
  emit("sub rax, %zu", var->offset);
  push(AX);
  comment("end Var %s", var->name);
}

void emit_node(Node* node) {
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
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    store(DI, size_of(type_of(node->lhs)));
    push(DI);
    comment("end NASSIGN");
    break;
  }
  case NPLUS:
    comment("start NPLUS");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    // TODO: regを使ってnode->lhsの型のサイズに合わせたコード生成
    emit("add %s, %s", reg64[AX], reg64[DI]);
    push(AX);
    comment("end NPLUS");
    break;
  case NMINUS:
    comment("start NMINUS");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    emit("sub %s, %s", reg64[AX], reg64[DI]);
    push(AX);
    comment("end NMINUS");
    break;
  case NMUL:
    comment("start NMUL");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    emit("imul %s", reg64[DI]);
    push(AX);
    comment("end NMUL");
    break;
  case NDIV:
    comment("start NDIV");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    emit("mov %s, %d", reg64[DX], 0);
    emit("div %s", reg64[DI]);
    push(AX);
    comment("end NDIV");
    break;
  case NEQ:
    comment("start NEQ");
    emit_node(node->lhs);
    emit_node(node->rhs);
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
    emit_node(node->lhs);
    emit_node(node->rhs);
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
    emit_node(node->lhs);
    emit_node(node->rhs);
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
    emit_node(node->lhs);
    emit_node(node->rhs);
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
    emit_node(node->lhs);
    emit_node(node->rhs);
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
    emit_node(node->lhs);
    emit_node(node->rhs);
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
      emit_node(node->args->ptr[i]);
    }
    for (ptrdiff_t i = node->args->length - 1; i >= 0; i--) {
      pop(argregs[i]);
    }

    push(R10);
    push(R11);
    emit("mov %s, %d", reg64[AX], 0);

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
  case NADDR: {
    comment("start NADDR");
    emit_lval(node->expr);
    comment("end NADDR");
    break;
  }
  case NDEREF: {
    comment("start NDEREF");
    emit_node(node->expr);
    pop(AX);
    load(AX, 8); // TODO: size_of(type_of(node))
    push(AX);
    comment("end NDEREF");
    break;
  }
  case NEXPR_STMT:
    comment("start NEXPR_STMT");
    emit_node(node->expr);
    comment("end NRETURN");
    break;
  case NRETURN:
    comment("start NRETURN");
    emit_node(node->expr);
    pop(AX);
    emit("jmp %s", func_end_label);
    comment("end NRETURN");
    break;
  case NIF: {
    comment("start NIF");
    emit_node(node->cond);
    pop(AX);
    emit("cmp %s, %d", reg64[AX], 0);
    char* l = new_label("end");
    emit("je %s", l);
    emit_node(node->then);
    printf("%s:\n", l);
    comment("end NIF");
    break;
  }
  case NIFELSE: {
    comment("start NIFELSE");
    emit_node(node->cond);
    pop(AX);
    emit("cmp %s, %d", reg64[AX], 0);
    char* els = new_label("else");
    emit("je %s", els);
    emit_node(node->then);
    char* end = new_label("end");
    emit("jmp %s", end);
    printf("%s:\n", els);
    emit_node(node->els);
    printf("%s:\n", end);
    comment("end NIFELSE");
    break;
  }
  case NWHILE: {
    comment("start NWHILE");
    char* begin = new_label("begin");
    char* end = new_label("end");
    printf("%s:\n", begin);
    emit_node(node->cond);
    pop(AX);
    emit("cmp %s, %d", reg64[AX], 0);
    emit("je %s", end);
    emit_node(node->body);
    emit("jmp %s", begin);
    printf("%s:\n", end);
    comment("end NWHILE");
    break;
  }
  case NFOR: {
    comment("start NFOR");
    char* begin = new_label("begin");
    char* end = new_label("end");
    emit_node(node->init);
    printf("%s:\n", begin);
    emit_node(node->cond);
    pop(AX);
    emit("cmp %s, %d", reg64[AX], 0);
    emit("je %s", end);
    emit_node(node->body);
    emit_node(node->step);
    emit("jmp %s", begin);
    printf("%s:\n", end);
    comment("end NFOR");
    break;
  }
  case NBLOCK: {
    comment("start NBLOCK");
    for (size_t i = 0; i < node->stmts->length; i++) {
      emit_node(node->stmts->ptr[i]);
    }
    comment("end NBLOCK");
    break;
  }
  default:
    eprintf("emit error: ");
    dump_node(node, 0);
    error(" unimplemented\n");
  }
}

void emit_function(Function* func) {
  comment("start Function");
  func_end_label = new_label("end");

  puts(".text");
  printf(".global %s\n", func->name);
  printf("%s:\n", func->name);

  emit("push rbp");
  emit("mov rbp, rsp");
  emit("sub rsp, %lu", func->local_size);
  stack_size += func->local_size;

  for (size_t i = 0; i < func->params->length; i++) {
    Var* param = func->params->ptr[i];
    emit_var(param);
    store(argregs[i], size_of(param->type));
  }

  emit_node(func->body);

  printf("%s:\n", func_end_label);
  emit("leave");
  emit("ret");

  comment("end Function");
}

void gen_x86(Program* prog) {
  puts(".intel_syntax noprefix");
  for (size_t i = 0; i < prog->funcs->length; i++) {
    emit_function(prog->funcs->ptr[i]);
  }
}
