#include "hoc.h"

static void set_last_use(IReg* reg, int last_use) {
  if (reg && reg->last_use < last_use) {
    reg->last_use = last_use;
  }
}

static void set_def(IReg* reg, int def) {
  if (reg) {
    reg->def = def;
  }
}

static Vector* collect_regs(IFunc* func) {
  int ic = 0;
  Vector* regs = new_vec();

  for (int i = 0; i < func->params->length; i++) {
    IReg* param = func->params->ptr[i];
    set_def(param, ic);
    set_last_use(param, ic);
    vec_push(regs, param);
  }

  for (int i = 0; i < func->blocks->length; i++) {
    Block* block = func->blocks->ptr[i];

    for (int j = 0; j < block->instrs->length; j++, ic++) {
      IR* inst = block->instrs->ptr[j];
      set_def(inst->r0, ic);
      set_last_use(inst->r1, ic);
      set_last_use(inst->r2, ic);

      if (inst->r0) {
        vec_push(regs, inst->r0);
      }
    }
  }

  return regs;
}

void alloc_regs(IProgram* prog) {
  for (int i = 0; i < prog->ifuncs->length; i++) {
    Vector* regs = collect_regs(prog->ifuncs->ptr[i]);

    for (int i = 0; i < regs->length; i++) {
      printf("%s ", show_ireg(regs->ptr[i]));
    }
    printf("\n");
  }
}
