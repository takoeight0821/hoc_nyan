#include "hoc.h"

typedef struct VarEnv {
  struct VarEnv* next;
  char* name;
  IReg* reg;
} VarEnv;

static int label_id;
static int reg_id;
static Block* current_block;
static char* break_label;
static Vector* blocks;
static VarEnv* var_env;

static void assign_var(char* name, IReg* reg) {
  VarEnv* new = calloc(1, sizeof(VarEnv));
  new->name = name;
  new->reg = reg;
  new->next = var_env;
  var_env = new;
}

static IReg* lookup_var(char* name) {
  for (VarEnv* env = var_env; env != NULL; env = env->next) {
    if (streq(env->name, name)) {
      return env->reg;
    }
  }
  return NULL;
}

static char* new_label(char* name) {
  char* label = format(".L%s%d", name, label_id);
  label_id++;
  return label;
}

static IReg* new_reg(size_t size) {
  IReg* new = calloc(1, sizeof(IReg));
  new->id = reg_id++;
  new->size = size;
  return new;
}

static Block* new_block(char* label) {
  Block* new = calloc(1, sizeof(Block));
  new->label = label;
  new->instrs = new_vec();
  return new;
}

static void in_new_block(char* label) {
  Block* block = new_block(label);
  vec_push(blocks, block);
  current_block = block;
}

static IR* imm(IReg* reg, int val) {
  IR* new = calloc(1, sizeof(IR));
  new->op = IIMM;
  new->r0 = reg;
  new->imm_int = val;
  return new;
}

static IR* label(IReg* reg, char* name) {
  IR* new = calloc(1, sizeof(IR));
  new->op = ILABEL;
  new->r0 = reg;
  new->label = name;
  return new;
}

static IR* bitwise_not(IReg* reg, IReg* val) {
  IR* new = new_ir(INOT);
  new->r0 = reg;
  new->r1 = val;
  return new;
}

static IR* alloc(IReg* reg, int size) {
  assert(reg->size == 8);
  IR* new = new_ir(IALLOC);
  new->r0 = reg;
  new->imm_int = size;
  return new;
}

static IR* load(IReg* dst, IReg* src) {
  IR* new = new_ir(ILOAD);
  new->r0 = dst;
  new->r1 = src;
  return new;
}

static IR* storearg(IReg* dst, int arg_index, size_t size) {
  assert(dst);
  IR* new = new_ir(ISTOREARG);
  new->r1 = dst;
  new->imm_int = arg_index;
  new->size = size;
  return new;
}

static IR* store(IReg* dst, IReg* src) {
  assert(dst);
  assert(src);
  IR* new = new_ir(ISTORE);
  new->r1 = dst;
  new->r2 = src;
  return new;
}

static IR* move(IReg* dst, IReg* src) {
  IR* new = new_ir(IMOV);
  new->r0 = dst;
  new->r1 = src;
  return new;
}

static IR* branch(IReg* cond, char* then_label, char* else_label) {
  IR* new = new_ir(IBR);
  new->r1 = cond;
  new->then = then_label;
  new->els = else_label;
  return new;
}

static IR* jmp(char* jump_to) {
  IR* new = new_ir(IJMP);
  new->jump_to = jump_to;
  return new;
}

static void emit_ir(IR* ir) {
  vec_push(current_block->instrs, ir);
}

static IReg* emit_expr(Node* node);

static IReg* emit_lval(Node* node) {
  switch (node->tag) {
  case NVAR: {
    return lookup_var(node->name);
  }
  case NGVAR: {
    IReg* addr = new_reg(8);
    emit_ir(label(addr, node->name));
    return addr;
  }
  case NMEMBER: {
    IReg* addr = emit_lval(node->expr);
    IReg* offset = new_reg(8);
    emit_ir(imm(offset, field_offset(type_of(node->expr)->fields, node->name)));
    IReg* reg = new_reg(8);
    emit_ir(new_binop_ir(IADD, reg, addr, offset));
    return reg;
  }
  case NDEREF: {
    return emit_expr(node->expr);
  }
  default: {
    bad_token(node->token, "gen_ir error: emit_lval");
  }
  }
  return NULL;
}

static IReg* emit_expr(Node* node) {
  switch (node->tag) {
  case NINT: {
    IReg* reg = new_reg(4);
    emit_ir(imm(reg, node->integer));
    return reg;
  }
  case NVAR: {
    IReg* addr = emit_lval(node);

    if (node->type->array_size == 0) {
      IReg* val = new_reg(size_of(type_of(node)));
      emit_ir(load(val, addr));
      return val;
    } else {
      return addr;
    }
  }
  case NGVAR: {
    IReg* addr = emit_lval(node);

    if (node->type->array_size == 0) {
      IReg* val = new_reg(size_of(type_of(node)));
      emit_ir(load(val, addr));
      return val;
    } else {
      return addr;
    }
  }
  case NASSIGN: {
    IReg* addr = emit_lval(node->lhs);
    IReg* val = emit_expr(node->rhs);
    IReg* val1 = new_reg(size_of(type_of(node->lhs)));
    emit_ir(move(val1, val));
    emit_ir(store(addr, val1));
    return val1;
  }
  case NADD: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(IADD, reg, lhs, rhs));
    return reg;
  }
  case NSUB: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(ISUB, reg, lhs, rhs));
    return reg;
  }
  case NMUL: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(IMUL, reg, lhs, rhs));
    return reg;
  }
  case NDIV: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(IDIV, reg, lhs, rhs));
    return reg;
  }
  case NMOD: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(IMOD, reg, lhs, rhs));
    return reg;
  }
  case NLT: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(ILT, reg, lhs, rhs));
    return reg;
  }
  case NLE: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(ILE, reg, lhs, rhs));
    return reg;
  }
  case NGT: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(IGT, reg, lhs, rhs));
    return reg;
  }
  case NGE: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(IGE, reg, lhs, rhs));
    return reg;
  }
  case NEQ: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(IEQ, reg, lhs, rhs));
    return reg;
  }
  case NNE: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(INE, reg, lhs, rhs));
    return reg;
  }
  case NLOGNOT: {
    IReg* val = emit_expr(node->expr);
    IReg* zero = new_reg(size_of(type_of(node)));
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(imm(zero, 0));
    emit_ir(new_binop_ir(IEQ, reg, val, zero));
    return reg;
  }
  case NNOT: {
    IReg* val = emit_expr(node->expr);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(bitwise_not(reg, val));
    return reg;
  }
  case NAND: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(IAND, reg, lhs, rhs));
    return reg;
  }
  case NOR: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(IOR, reg, lhs, rhs));
    return reg;
  }
  case NXOR: {
    IReg* lhs = emit_expr(node->lhs);
    IReg* rhs = emit_expr(node->rhs);
    IReg* reg = new_reg(size_of(type_of(node)));
    emit_ir(new_binop_ir(IXOR, reg, lhs, rhs));
    return reg;
  }
  case NLOGAND: {
    char* when_lhs_false = new_label("when_lhs_false");
    char* when_lhs_true = new_label("when_lhs_true");
    char* end = new_label("end");

    IReg* lhs = emit_expr(node->lhs);
    IReg* zero = new_reg(size_of(type_of(node->lhs)));
    emit_ir(imm(zero, 0));
    IReg* lhs_cond = new_reg(size_of(type_of(node->lhs)));
    emit_ir(new_binop_ir(IEQ, lhs_cond, lhs, zero));
    emit_ir(branch(lhs_cond, when_lhs_false, when_lhs_true));

    IReg* reg = new_reg(size_of(type_of(node)));

    in_new_block(when_lhs_true);
    IReg* rhs = emit_expr(node->rhs);
    emit_ir(move(reg, rhs));
    emit_ir(jmp(end));

    in_new_block(when_lhs_false);
    emit_ir(imm(reg, 0));
    emit_ir(jmp(end));

    in_new_block(end);
    return reg;
  }
  case NLOGOR: {
    char* when_lhs_false = new_label("when_lhs_false");
    char* when_lhs_true = new_label("when_lhs_true");
    char* end = new_label("end");

    IReg* lhs = emit_expr(node->lhs);
    IReg* zero = new_reg(size_of(type_of(node->lhs)));
    emit_ir(imm(zero, 0));
    IReg* lhs_cond = new_reg(size_of(type_of(node->lhs)));
    emit_ir(new_binop_ir(IEQ, lhs_cond, lhs, zero));
    emit_ir(branch(lhs_cond, when_lhs_false, when_lhs_true));

    IReg* reg = new_reg(size_of(type_of(node)));

    in_new_block(when_lhs_true);
    emit_ir(move(reg, lhs));
    emit_ir(jmp(end));

    in_new_block(when_lhs_false);
    IReg* rhs = emit_expr(node->rhs);
    emit_ir(move(reg, rhs));
    emit_ir(jmp(end));

    in_new_block(end);
    return reg;
  }
  case NCOMMA: {
    emit_expr(node->lhs);
    return emit_expr(node->rhs);
  }
  case NDEFVAR: {
    // for initializer
    IReg* new = new_reg(8);
    emit_ir(alloc(new, size_of(type_of(node))));
    assign_var(node->name, new);
    return NULL;
  }
  case NCALL: {
    Vector* args = new_vec();

    for (size_t i = 0; i < node->args->length; i++) {
      IReg* arg = emit_expr(node->args->ptr[i]);
      vec_push(args, arg);
    }

    IR* ir = new_ir(ICALL);
    ir->r0 = new_reg(size_of(type_of(node)));
    ir->func_name = node->name;
    ir->args = args;
    emit_ir(ir);

    return ir->r0;
  }
  case NADDR: {
    return emit_lval(node->expr);
  }
  case NDEREF: {
    IReg* reg = new_reg(size_of(type_of(node)));
    IReg* addr = emit_expr(node->expr);
    emit_ir(load(reg, addr));
    return reg;
  }
  case NMEMBER: {
    IReg* reg = new_reg(size_of(type_of(node)));
    IReg* addr = emit_lval(node);
    emit_ir(load(reg, addr));
    return reg;
  }
  case NSIZEOF: {
    IReg* reg = new_reg(size_of(type_of(node->expr)));
    emit_ir(imm(reg, size_of(type_of(node->expr))));
    return reg;
  }
  case NCAST: {
    IReg* reg = new_reg(size_of(type_of(node->expr)));
    emit_ir(move(reg, emit_expr(node->expr)));
    return reg;
  }
  default: {
    bad_token(node->token, "error: statement cannot appear on here");
  }
  }
}

static void emit_stmt(Node* node) {
  switch (node->tag) {
  case NDEFVAR: {
    IReg* new = new_reg(8);
    emit_ir(alloc(new, size_of(type_of(node))));
    assign_var(node->name, new);
    break;
  }
  case NEXPR_STMT: {
    emit_expr(node->expr);
    break;
  }
  case NRETURN: {
    if (node->expr) {
      IReg* val = emit_expr(node->expr);
      IR* new = calloc(1, sizeof(IR));
      new->op = IRET;
      new->r1 = val;
      emit_ir(new);
    } else {
      IR* new = calloc(1, sizeof(IR));
      new->op = IRET;
      emit_ir(new);
    }
    break;
  }
  case NBLOCK: {
    // FIXME: スコープを正しく切る
    VarEnv* prev = var_env; 
    for (int i = 0; i < node->stmts->length; i++) {
      emit_stmt(node->stmts->ptr[i]);
    }
    var_env = prev;
    break;
  }
  case NIF: {
    IReg* cond = emit_expr(node->cond);

    char* then_label = new_label("then");
    char* end_label = new_label("endif");

    emit_ir(branch(cond, then_label, end_label));

    in_new_block(then_label);
    emit_stmt(node->then);

    in_new_block(end_label);
    break;
  }
  case NIFELSE: {
    IReg* cond = emit_expr(node->cond);

    char* then_label = new_label("then");
    char* else_label = new_label("else");
    char* end_label = new_label("endif");

    emit_ir(branch(cond, then_label, else_label));

    in_new_block(then_label);
    emit_stmt(node->then);
    emit_ir(jmp(end_label));

    in_new_block(else_label);
    emit_stmt(node->els);

    in_new_block(end_label);
    break;
  }
  case NCOMMA: {
    // for initializer
    warn_token(node->token, "warning(expr_stmt): NCOMMA");
    emit_expr(node->lhs);
    emit_expr(node->rhs);
    break;
  }
  case NWHILE: {
    char* begin = new_label("begin");
    char* end = new_label("end");
    char* prev_break = break_label;
    break_label = end;

    in_new_block(begin);
    IReg* cond = emit_expr(node->cond);
    IReg* zero = new_reg(size_of(type_of(node->cond)));
    emit_ir(imm(zero, 0));
    IReg* cond1 = new_reg(size_of(type_of(node->cond)));
    emit_ir(new_binop_ir(IEQ, cond1, cond, zero));
    char* body = new_label("body");
    emit_ir(branch(cond1, end, body));

    in_new_block(body);
    emit_stmt(node->body);
    emit_ir(jmp(begin));

    in_new_block(end);
    break_label = prev_break;
    break;
  }
  case NFOR: {
    char* begin = new_label("begin");
    char* end = new_label("end");
    char* prev_break = break_label;
    break_label = end;

    emit_expr(node->init);

    in_new_block(begin);
    IReg* cond = emit_expr(node->cond);
    IReg* cond1 = new_reg(size_of(type_of(node->cond)));
    IReg* zero = new_reg(size_of(type_of(node->cond)));
    emit_ir(imm(zero, 0));
    IR* c = new_binop_ir(IEQ, cond1, cond, zero);

    char* body = new_label("body");
    emit_ir(branch(cond1, end, body));

    in_new_block(body);
    emit_stmt(node->body);
    emit_expr(node->step);
    emit_ir(jmp(end));

    in_new_block(end);
    break_label = prev_break;

    break;
  }
  case NSWITCH: {
    IReg* val = emit_expr(node->expr);

    for (size_t i = 0; i < node->cases->length; i++) {
      Node* clause = node->cases->ptr[i];

      if (clause->tag == NDEFAULT) {
        emit_ir(jmp(clause->name));
      } else {
        IReg* c_val = emit_expr(clause->expr);
        IReg* cond = new_reg(4);
        emit_ir(new_binop_ir(IEQ, cond, val, c_val));
        char* next = new_label("next");
        emit_ir(branch(cond, clause->name, next));
        in_new_block(next);
      }
    }
    char* prev_break = break_label;
    break_label = new_label("break");
    emit_stmt(node->body);

    in_new_block(break_label);
    break_label = prev_break;
    
    break;
  }
  case NCASE: {
    in_new_block(node->name);
    emit_stmt(node->body);
    break;
  }
  case NDEFAULT: {
    in_new_block(node->name);
    emit_stmt(node->body);
    break;
  }
  case NBREAK: {
    assert(break_label);
    emit_ir(jmp(break_label));
    break;
  }
  case NINT: case NVAR: case NGVAR:
  case NADD: case NSUB: case NMUL: case NDIV: case NMOD:
  case NLT: case NLE: case NGT: case NGE: case NEQ: case NNE:
  case NLOGNOT: case NNOT: case NAND: case NOR: case NXOR:
  case NLOGAND: case NLOGOR: case NASSIGN: case NCALL:
  case NADDR: case NDEREF: case NMEMBER: case NSIZEOF: case NCAST:
    bad_token(node->token, "error: expression cannot appear on here");
  }
}

static IFunc* emit_func(Function* func) {
  IFunc* ifunc = calloc(1, sizeof(IFunc));
  ifunc->name = func->name;
  ifunc->is_static = func->is_static;
  ifunc->has_va_arg = func->has_va_arg;
  ifunc->params = new_vec();

  if (!func->body) {
    return ifunc;
  }

  ifunc->blocks = new_vec();
  blocks = ifunc->blocks;

  ifunc->entry_label = new_label("entry");
  in_new_block(ifunc->entry_label);

  for (int i = 0; i < func->params->length; i++) {
    IReg* reg = new_reg(8);
    emit_ir(alloc(reg, size_of(type_of(func->params->ptr[i]))));
    emit_ir(storearg(reg, i, size_of(type_of(func->params->ptr[i]))));
    vec_push(ifunc->params, reg);
    assign_var(((Node*)func->params->ptr[i])->name, reg);
  }

  emit_stmt(func->body);

  return ifunc;
}

IFunc* is_defined_proto(Vector* funcs, char* name) {
  for (size_t i = 0; i < funcs->length; i++) {
    IFunc* func = funcs->ptr[i];
    if (streq(func->name, name) && !func->blocks) {
      return func;
    }
  }
  return false;
}

IFunc* is_defined(Vector* funcs, char* name) {
  for (size_t i = 0; i < funcs->length; i++) {
    IFunc* func = funcs->ptr[i];
    if (streq(func->name, name) && func->blocks) {
      return func;
    }
  }
  return false;
}

IProgram* gen_ir(Program* program) {
  IProgram* ip = calloc(1, sizeof(IProgram));
  ip->globals = program->globals;
  ip->ifuncs = new_vec();

  for (int i = 0; i < program->funcs->length; i++) {
    Function* func = program->funcs->ptr[i];
    IFunc* ifunc = emit_func(func);

    // プロトタイプ宣言を上書きする。
    // gen_x86ですべてのプロトタイプ宣言を外部関数だと解釈する
    IFunc* proto = is_defined_proto(ip->ifuncs, func->name);
    if (proto) {
      memcpy(proto, ifunc, sizeof(IFunc));
    } else {
      vec_push(ip->ifuncs, ifunc);
    }
  }

  return ip;
}
