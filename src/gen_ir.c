#include "hoc.h"

typedef struct VarEnv {
  struct VarEnv* next;
  char* name;
  IReg* reg;
} VarEnv;

static int label_id;
static int reg_id;
static Block* current_block;
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

static IR* ret(IReg* val) {
  IR* new = calloc(1, sizeof(IR));
  new->op = IRET;
  new->r1 = val;
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

static IR* store(IReg* dst, IReg* src) {
  IR* new = new_ir(ISTORE);
  new->r1 = dst;
  new->r2 = src;
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
    IReg* val = new_reg(size_of(type_of(node)));
    emit_ir(load(val, addr));
    return val;
  }
  case NGVAR: {
    IReg* addr = emit_lval(node);
    IReg* val = new_reg(size_of(type_of(node)));
    emit_ir(load(val, addr));
    return val;
  }
  case NASSIGN: {
    IReg* addr = emit_lval(node->lhs);
    IReg* val = emit_expr(node->rhs);
    emit_ir(store(addr, val));
    return val;
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
  default: {
    dump_node(node, 0);
    error("unimplemented: emit_expr\n");
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
    IReg* val = emit_expr(node->expr);
    emit_ir(ret(val));
    break;
  }
  case NBLOCK: {
    for (int i = 0; i < node->stmts->length; i++) {
      emit_stmt(node->stmts->ptr[i]);
    }
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
  default: {
    error("unimplemented: emit_stmt\n");
  }
  }
}

static IFunc* emit_func(Function* func) {
  IFunc* ifunc = calloc(1, sizeof(IFunc));
  ifunc->name = func->name;
  ifunc->is_static = func->is_static;
  ifunc->has_va_arg = func->has_va_arg;
  ifunc->params = new_vec();

  if (!func->body) {
    ifunc->blocks = new_vec();
    blocks = ifunc->blocks;

    for (int i = 0; i < func->params->length; i++) {
      IReg* reg = new_reg(size_of(type_of(func->params->ptr[i])));
      vec_push(ifunc->params, reg);
      assign_var(((Node*)func->params->ptr[i])->name, reg);
    }

    ifunc->entry_label = new_label("entry");
    in_new_block(ifunc->entry_label);
    emit_stmt(func->body);
  }

  return ifunc;
}

IProgram* gen_ir(Program* program) {
  IProgram* ip = calloc(1, sizeof(IProgram));
  ip->globals = program->globals;
  ip->ifuncs = new_vec();

  for (int i = 0; i < program->funcs->length; i++) {
    vec_push(ip->ifuncs, emit_func(program->funcs->ptr[i]));
  }

  return ip;
}
