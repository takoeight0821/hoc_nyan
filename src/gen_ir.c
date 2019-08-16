#include <hoc.h>

static int label_id;
static int reg_id;
static Block* out;
static Vector* blocks;

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
  }
}

static void emit_stmt(Node* node) {
  switch (node->tag) {
  case NEXPR_STMT: {
    emit_expr(node);
  }
  case NRETURN: {
    IReg* val = emit_expr(node->expr);
    emit_ir(ret(val));
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
    vec_push(ifunc->params, new_reg());
  }

  Block* entry = new_block(new_label("entry"));
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
