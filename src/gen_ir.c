#include "hoc.h"

typedef struct VarEnv {
  struct VarEnv* next;
  char* name;
  IReg* reg;
} VarEnv;

static int label_id;
static int reg_id;
static Block* out;
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
  char* label = format(".L%s", name);
  label_id++;
  return label;
}

static IReg* new_reg(void) {
  IReg* new = calloc(1, sizeof(IReg));
  new->id = reg_id++;
  return new;
}

static Block* new_block(char* label) {
  Block* new = calloc(1, sizeof(Block));
  new->label = label;
  new->instrs = new_vec();
  return new;
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
  new->r0 = val;
  return new;
}

static IR* label(IReg* reg, char* name) {
  IR* new = calloc(1, sizeof(IR));
  new->op = ILABEL;
  new->r0 = reg;
  new->label = name;
  return new;
}

static void emit_ir(IR* ir) {
  vec_push(out->instrs, ir);
}

static IReg* emit_expr(Node* node) {
  switch (node->tag) {
  case NINT: {
    IReg* reg = new_reg();
    emit_ir(imm(reg, node->integer));
    return reg;
  }
  case NVAR: {
    return lookup_var(node->name);
  }
  case NGVAR: {
    IReg* reg = new_reg();
    emit_ir(label(reg, node->name));
    return reg;
  }
  }
}

static void emit_stmt(Node* node) {
  switch (node->tag) {
  case NDEFVAR: {
    IReg* new = new_reg();
    assign_var(node->name, new);
    break;
  }
  case NEXPR_STMT: {
    emit_expr(node);
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
  }
}

static IFunc* emit_func(Function* func) {
  IFunc* ifunc = calloc(1, sizeof(IFunc));
  ifunc->name = func->name;
  ifunc->params = new_vec();
  ifunc->blocks = new_vec();
  blocks = ifunc->blocks;

  for (int i = 0; i < func->params->length; i++) {
    IReg* reg = new_reg();
    vec_push(ifunc->params, reg);
    assign_var(((Node*)func->params->ptr[i])->name, reg);
  }

  Block* entry = new_block(new_label("entry"));
  ifunc->entry_label = entry->label;
  out = entry;
  vec_push(blocks, entry);
  emit_stmt(func->body);
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
