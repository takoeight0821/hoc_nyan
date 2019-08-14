#include "hoc.h"

static char* reg64[9] = { "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11" };
static char* reg32[9] = { "eax", "edi", "esi", "edx", "ecx", "r8d", "r9d", "r10d", "r11d" };
static char* reg8[9] = { "al", "dil", "sil", "dl", "cl", "r8b", "r9b", "r10b", "r11b" };
static Reg argregs[6] = {DI, SI, DX, CX, R8, R9};
static int label_id;
static int stack_size;
static char* func_end_label;
static char* break_label;
static int numgp;
#define REGAREA_SIZE 176

static char* reg(Reg r, size_t s) {
  if (s == 1) {
    return reg8[r];
  } else if (s == 4) {
    return reg32[r];
  } else {
    return reg64[r];
  }
}

static char* new_label(char* name) {
  return format(".L%s%u", name, label_id++);
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

#ifndef __hoc__
__attribute__((format(printf, 1, 2))) static void comment(char *fmt, ...);
#endif

static void comment(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  printf("# ");
  vprintf(fmt, ap);
  printf("\n");
}

static void push(Reg src) {
  printf("\tpush %s\n", reg64[src]);
  stack_size += 8;
}

static void pushi(int src) {
  printf("\tpush %d\n", src);
  stack_size += 8;
}

static void pushstring(char* l) {
  emit("lea rax, %s", l);
  push(AX);
}

static void pop(Reg dst) {
  printf("\tpop %s\n", reg64[dst]);
  stack_size -= 8;
}

static void load(Reg dst, size_t size) {
  if (size == 1) {
    // 8bitの値は自動では0拡張されないので、movzxを使う
    // 32bit整数との演算を行うので32bitレジスタにロードする
    emit("movsx %s, BYTE PTR [rax]", reg(dst, 4));
  } else {
   emit("mov %s, [rax]", reg(dst, size));
  }
}

static void store(Reg src, size_t size) {
  emit("mov [rax], %s", reg(src, size));
}

static void emit_node(Node* node);

static void emit_var(Node* var) {
  comment("start lval NVAR %s", var->name);
  emit("lea rax, -%zu[rbp]", var->offset);
  push(AX);
  comment("end lval NVAR %s", var->name);
}

static void emit_lval(Node* node) {
  if (node->tag == NVAR) {
    emit_var(node);
  } else if (node->tag == NGVAR) {
    comment("start lval NGVAR");
    emit("lea rax, %s", node->name);
    push(AX);
    comment("end lval NGVAR");
  } else if (node->tag == NMEMBER) {
    comment("start lval NMEMBER");
    emit_lval(node->expr);
    size_t offset = field_offset(type_of(node->expr)->fields, node->name);
    pop(AX);
    emit("lea rax, %zu[rax]", offset);
    push(AX);
    comment("end lval NMEMBER");
  } else if (node->tag == NDEREF){
    comment("start lval NDEREF");
    emit_node(node->expr);
    comment("end lval NDEREF");
  } else {
    bad_token(node->token, "emit error: emit_lval");
  }
}

static void emit_assign(Node* lhs, Node* rhs) {
  comment("  start lval");
  emit_lval(lhs);
  comment("  end lval");
  emit_node(rhs);
  pop(DI);
  pop(AX);
  store(DI, size_of(type_of(lhs)));
  push(DI);
}

static void emit_builtin_va_start(Node* node) {
  emit_node(node);
  pop(AX); // address of va_list
  push(CX); // save register
  emit("mov DWORD PTR [rax], %d", numgp * 8);
  emit("mov DWORD PTR [rax + 4], 48");
  emit("lea rcx, [rbp - %d]", REGAREA_SIZE);
  emit("mov [rax + 16], rcx");
  pop(CX); // load register
}

static void emit_node(Node* node) {
  // nodeがNULLなら何もしない
  // for(;;)とかが該当
  if (!node) {
    return;
  }
  switch (node->tag) {
  case NINT:
    comment("start NINT");
    pushi(node->integer);
    comment("end NINT");
    break;
  case NVAR: {
    comment("start NVAR");
    emit_lval(node);

    if (node->type->array_size == 0) {
      pop(AX);
      load(AX, size_of(type_of(node)));
      push(AX);
    } else {
      // nodeが配列型の変数の場合、lvalとしてコンパイルする（配列の先頭へのポインタになる）
      comment("emit array var");
    }

    comment("end NVAR");
    break;
  }
  case NGVAR: {
    comment("start NGVAR");
    emit_lval(node);

    if (node->type->array_size == 0) {
      pop(AX);
      load(AX, size_of(type_of(node)));
      push(AX);
    } else {
      // nodeが配列型の変数の場合、lvalとしてコンパイルする（配列の先頭へのポインタになる）
      comment("emit array var");
    }

    comment("end NGVAR");
    break;
  }
  case NADD: {
    comment("start NADD");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    size_t size = size_of(type_of(node));
    emit("add %s, %s", reg(AX, size), reg(DI, size));
    push(AX);
    comment("end NADD");
    break;
  }
  case NSUB: {
    comment("start NSUB");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    size_t size = size_of(type_of(node));
    emit("sub %s, %s", reg(AX, size), reg(DI, size));
    push(AX);
    comment("end NSUB");
    break;
  }
  case NMUL: {
    comment("start NMUL");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    size_t size = size_of(type_of(node->lhs));
    emit("imul %s", reg(DI, size));
    push(AX);
    comment("end NMUL");
    break;
  }
  case NDIV: {
    comment("start NDIV");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    emit("mov %s, %d", reg64[DX], 0);
    size_t size = size_of(type_of(node->lhs));
    emit("div %s", reg(DI, size));
    push(AX);
    comment("end NDIV");
    break;
  }
  case NMOD: {
    comment("start NMOD");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    emit("mov %s, %d", reg64[DX], 0);
    size_t size = size_of(type_of(node->lhs));
    emit("div %s", reg(DI, size));
    push(DX);
    comment("end NMOD");
    break;
  }
  case NLT: {
    comment("start NLT");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    size_t size = size_of(type_of(node->lhs));
    emit("cmp %s, %s", reg(AX, size), reg(DI, size));
    emit("setl al");
    emit("movzx rax, al");
    push(AX);
    comment("end NLT");
    break;
  }
  case NLE: {
    comment("start NLE");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    size_t size = size_of(type_of(node->lhs));
    emit("cmp %s, %s", reg(AX, size), reg(DI, size));
    emit("setle al");
    emit("movzx rax, al");
    push(AX);
    comment("end NLE");
    break;
  }
  case NGT: {
    comment("start NGT");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    size_t size = size_of(type_of(node->lhs));
    emit("cmp %s, %s", reg(AX, size), reg(DI, size));
    emit("setg al");
    emit("movzx rax, al");
    push(AX);
    comment("end NGT");
    break;
  }
  case NGE: {
    comment("start NGE");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    size_t size = size_of(type_of(node->lhs));
    emit("cmp %s, %s", reg(AX, size), reg(DI, size));
    emit("setge al");
    emit("movzx rax, al");
    push(AX);
    comment("end NGE");
    break;
  }
  case NEQ: {
    comment("start NEQ");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    size_t size = size_of(type_of(node->lhs));
    emit("cmp %s, %s", reg(AX, size), reg(DI, size));
    emit("sete al");
    emit("movzx rax, al");
    push(AX);
    comment("end NEQ");
    break;
  }
  case NNE: {
    comment("start NNE");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    size_t size = size_of(type_of(node->lhs));
    emit("cmp %s, %s", reg(AX, size), reg(DI, size));
    emit("setne al");
    emit("movzx rax, al");
    push(AX);
    comment("end NNE");
    break;
  }
  case NNOT: {
    comment("start NNOT");
    emit_node(node->expr);
    pop(AX);
    emit("cmp %s, 0", reg(AX, 4));
    emit("sete al");
    emit("movzx rax, al");
    push(AX);
    comment("end NNOT");
    break;
  }
  case NAND: {
    comment("start NAND");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    emit("and %s, %s", reg(AX, size_of(type_of(node))), reg(DI, size_of(type_of(node))));
    push(AX);
    comment("end NAND");
    break;
  }
  case NOR: {
    comment("start NOR");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    emit("or %s, %s", reg(AX, size_of(type_of(node))), reg(DI, size_of(type_of(node))));
    push(AX);
    comment("end NOR");
    break;
  }
  case NXOR: {
    comment("start NXOR");
    emit_node(node->lhs);
    emit_node(node->rhs);
    pop(DI);
    pop(AX);
    emit("xor %s, %s", reg(AX, size_of(type_of(node))), reg(DI, size_of(type_of(node))));
    push(AX);
    comment("end NXOR");
    break;
  }
  case NLOGAND: {
    comment("start NLOGAND");
    char* when_false = new_label("when_false");
    char* when_true = new_label("when_true");

    emit_node(node->lhs);
    pop(AX);
    emit("cmp %s, 0", reg(AX, size_of(type_of(node->lhs))));
    emit("je %s", when_false);

    emit_node(node->rhs);
    pop(AX);
    emit("cmp %s, 0", reg(AX, size_of(type_of(node->rhs))));
    emit("je %s", when_false);

    emit("mov rax, 1");
    emit("jmp %s", when_true);

    printf("%s:\n", when_false);
    emit("mov rax, 0");
    printf("%s:\n", when_true);
    push(AX);
    comment("end NLOGAND");
    break;
  }
  case NLOGOR: {
    comment("start NLOGAND");
    char* when_false = new_label("when_false");
    char* when_true = new_label("when_true");
    char* next = new_label("next");

    emit_node(node->lhs);
    pop(AX);
    emit("cmp %s, 0", reg(AX, size_of(type_of(node->lhs))));
    emit("jne %s", when_true);

    emit_node(node->rhs);
    pop(AX);
    emit("cmp %s, 0", reg(AX, size_of(type_of(node->rhs))));
    emit("je %s", when_false);

    printf("%s:\n", when_true);
    emit("mov rax, 1");
    push(AX);
    emit("jmp %s", next);

    printf("%s:\n", when_false);
    emit("mov rax, 0");
    push(AX);

    printf("%s:\n", next);
    comment("end NLOGAND");
    break;
  }
  case NCOMMA: {
    comment("start NCOMMA");
    emit_node(node->lhs);
    // pop unused value
    if (node->lhs->type != NULL && node->lhs->type->ty != TY_VOID) {
      pop(AX);
    }
    emit_node(node->rhs);
    comment("end NCOMMA");
    break;
  }
  case NDEFVAR: {
    break;
  }
  case NASSIGN: {
    comment("start NASSIGN");
    emit_assign(node->lhs, node->rhs);
    comment("end NASSIGN");
    break;
  }
  case NCALL: {
    comment("start NCALL");

    // スタックをがりがりいじりながらコード生成してるので、
    // rspのアライメントをうまいこと扱う必要がある。
    int pad = stack_size % 16;
    if (pad) {
      emit("sub rsp, %d", pad);
      stack_size += pad;
    }

    // builtinを特別あつかい
    if (streq("__hoc_builtin_va_start", node->name)) {
      comment("start __hoc_builtin_va_start");
      emit_builtin_va_start(node->args->ptr[0]);
      comment("end __hoc_builtin_va_start");
      break;
    }

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

    emit("call %s", node->name);
    pop(R11);
    pop(R10);

    if (pad) {
      emit("add rsp, %d", pad);
      stack_size -= pad;
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
    load(AX, size_of(node->expr->type->ptr_to));
    push(AX);
    comment("end NDEREF");
    break;
  }
  case NMEMBER: {
    comment("start NMEMBER");
    emit_lval(node);
    pop(AX);
    load(AX, size_of(node->type));
    push(AX);
    comment("end NMEMBER");
    break;
  }
  case NEXPR_STMT:
    comment("start NEXPR_STMT");
    emit_node(node->expr);
    if (node->expr->type != NULL || node->expr->type->ty != TY_VOID) {
      pop(AX);
    }
    comment("end NEXPR_STMT");
    break;
  case NRETURN:
    comment("start NRETURN");
    if (node->expr) {
      emit_node(node->expr);
      pop(AX);
    }
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
  case NSIZEOF: {
    comment("start NSIZEOF");
    pushi(size_of(type_of(node->expr)));
    comment("end NSIZEOF");
    break;
  }
  case NSTRING: {
    comment("start NSTRING");
    /* pushlabel(format(".string_%zu", node->str_id)); */
    pushstring(format(".string_%zu", node->str_id));
    comment("end NSTRING");
    break;
  }
  case NSWITCH: {
    comment("start NSWITCH");
    emit_node(node->expr);

    Node* clause;
    for (int i = 0; i < node->cases->length; i++) {
      clause = node->cases->ptr[i];

      if (clause->tag == NDEFAULT) {
        emit("jmp %s", clause->name);
      } else {
       emit_node(clause->expr); // TODO: emit_const
       pop(DI);
       pop(AX);
       push(AX);
       size_t size = size_of(type_of(node->expr));
       emit("cmp %s, %s", reg(AX, size), reg(DI, size));
       emit("je %s", clause->name);
      }
    }
    pop(AX); // pop node->expr

    char* prev_break = break_label;
    break_label = new_label("break");

    emit_node(node->body);

    printf("%s:\n", break_label);
    break_label = prev_break;
    comment("end NSWITCH");
    break;
  }
  case NCASE: {
    comment("start NCASE");
    printf("%s:\n", node->name);
    emit_node(node->body);
    if (type_of(node->body) != NULL && type_of(node->body)->ty != TY_VOID) {
      pop(AX); // pop unused value
    }
    comment("end NCASE");
    break;
  }
  case NDEFAULT: {
    comment("start NDEFAULT");
    printf("%s:\n", node->name);
    emit_node(node->body);
    if (type_of(node->body) != NULL && type_of(node->body)->ty != TY_VOID) {
      pop(AX); // pop unused value
    }
    comment("end NDEFAULT");
    break;
  }
  case NBREAK: {
    comment("start NBREAK");
    emit("jmp %s", break_label);
    comment("end NBREAK");
    break;
  }
  case NCAST: {
    comment("start NCAST");
    emit_node(node->expr);
    comment("end NCAST");
    break;
  }
  }
}

static void set_reg_nums(Vector* params) {
  numgp = params->length;
}

static int emit_regsave_area(void) {
  emit("sub rsp, %d", REGAREA_SIZE);
  emit("mov [rsp], rdi");
  emit("mov [rsp + 8], rsi");
  emit("mov [rsp + 16], rdx");
  emit("mov [rsp + 24], rcx");
  emit("mov [rsp + 32], r8");
  emit("mov [rsp + 40], r9");
  /* emit("movaps [rsp + 48], xmm0"); */
  /* emit("movaps [rsp + 64], xmm1"); */
  /* emit("movaps [rsp + 80], xmm2"); */
  /* emit("movaps [rsp + 96], xmm3"); */
  /* emit("movaps [rsp + 112], xmm4"); */
  /* emit("movaps [rsp + 128], xmm5"); */
  /* emit("movaps [rsp + 144], xmm6"); */
  /* emit("movaps [rsp + 160], xmm7"); */
  return REGAREA_SIZE;
}

static void emit_function(Function* func) {
  comment("start Function");

  if (func->body == NULL) {
    comment("prototype");
    comment("end Function");
    return;
  }

  func_end_label = new_label("end");

  if (!func->is_static) {
    printf(".global %s\n", func->name);
  }

  printf("%s:\n", func->name);
  emit("push rbp");
  emit("mov rbp, rsp");

  // variable arguments list
  if (func->has_va_arg) {
    set_reg_nums(func->params);
    stack_size += emit_regsave_area();
  }

  emit("sub rsp, %lu", func->local_size);
  stack_size += func->local_size + 8;

  for (size_t i = 0; i < func->params->length; i++) {
    Node* param = func->params->ptr[i];
    emit_var(param);
    store(argregs[i], size_of(param->type));
  }

  emit_node(func->body);

  printf("%s:\n", func_end_label);
  emit("leave");
  emit("ret");

  comment("end Function");
}

static void emit_const(Type* type, Node* node) {
  if (!node) {
    return;
  }

  switch (node->tag) {
  case NINT: {
    if (size_of(type) == 1) {
      emit(".byte %d", node->integer);
    } else if (size_of(type) == 4) {
      emit(".int %d", node->integer);
    } else if (size_of(type) == 8) {
      emit(".quad %d", node->integer);
    } else {
      bad_token(node->token, "emit error: emit_const(invalid size)");
    }
    break;
  }
  case NSTRING: {
    emit(".quad .string_%zu", node->str_id);
    break;
  }
  default:
    bad_token(node->token, "emit error: value is not constant");
  }
}

void gen_x86(Program* prog) {
  puts(".intel_syntax noprefix");

  puts(".data");
  for (size_t i = 0; i < prog->strs->length; i++) {
    printf(".string_%zu:\n", i);
    emit(".string \"%s\"", (char*)prog->strs->ptr[i]);
  }
  for (GVar* gvar = prog->globals; gvar != NULL; gvar = gvar->next) {
    if (gvar->init != NULL && !gvar->is_extern) {
      printf("%s:\n", gvar->name);
      emit_const(gvar->type, gvar->init);
    } else if (gvar->inits != NULL && !gvar->is_extern) {
      printf("%s:\n", gvar->name);
      for (size_t i = 0; i < gvar->inits->length; i++) {
        emit_const(gvar->type->ptr_to, gvar->inits->ptr[i]);
      }
    }
  }

  puts(".bss");
  for (GVar* gvar = prog->globals; gvar != NULL; gvar = gvar->next) {
    if (gvar->init == NULL && gvar->inits == NULL && !gvar->is_extern) {
      printf("%s:\n", gvar->name);
      emit(".zero %zu", size_of(gvar->type));
    }
  }

  puts(".text");
  for (size_t i = 0; i < prog->funcs->length; i++) {
    emit_function(prog->funcs->ptr[i]);
  }
}
