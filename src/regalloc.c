#include "hoc.h"

// Rewrite `A = B op C` to `A = B; A = A op C`.
static void three_to_two(Block* block) {
  Vector* v = new_vec();

  for (size_t i = 0; i < block->instrs->length; i++) {
    IR* inst = block->instrs->ptr[i];

    if (inst->r0 && inst->r1 && inst->r2) {
      IR* ir2 = new_ir(IMOV);
      ir2->r0 = inst->r0;
      ir2->r1 = inst->r1;
      vec_push(v, ir2);
      inst->r1 = inst->r0;
    }
    vec_push(v, inst);
  }

  block->instrs = v;
}

static void set_last_use(IReg* reg, int last_use) {
  if (reg && reg->last_use < last_use) {
    reg->last_use = last_use;
  }
}

static void set_def(IReg* reg, int def) {
  if (reg && reg->def == 0) {
    reg->def = def;
  }
}

static bool collected(Vector* regs, IReg* reg) {
  for (size_t i = 0; i < regs->length; i++) {
    IReg* elem = regs->ptr[i];
    if (reg->id == elem->id) {
      return true;
    }
  }
  return false;
}

static Vector* collect_regs(IFunc* func) {
  int ic = 0;
  Vector* regs = new_vec();

  for (int i = 0; i < func->blocks->length; i++) {
    Block* block = func->blocks->ptr[i];
    three_to_two(block);

    for (int j = 0; j < block->instrs->length; j++, ic++) {
      IR* inst = block->instrs->ptr[j];
      set_def(inst->r0, ic);
      set_last_use(inst->r1, ic);
      set_last_use(inst->r2, ic);

      if (inst->args) {
        for (size_t i = 0; i < inst->args->length; i++) {
          set_last_use(inst->args->ptr[i], ic);
        }
      }

      if (inst->r0 && !collected(regs, inst->r0)) {
        vec_push(regs, inst->r0);
      }
    }
  }

  return regs;
}

static int choose_to_spill(IReg **used) {
  int k = 0;
  for (int i = 1; i < NUM_REGS; i++) {
    if (used[k]->last_use < used[i]->last_use) {
      k = i;
    }
  }
  return k;

}

void scan(Vector* regs) {
  IReg** used = calloc(NUM_REGS, sizeof(IReg*));

  for (int i = 0; i < regs->length; i++) {
    IReg* reg = regs->ptr[i];
    bool found = false;

    for (int i = 0; i < NUM_REGS - 1; i++) {
      if (!(used[i] && reg->def < used[i]->last_use)) {
        assert(i < NUM_REGS);
        reg->real_reg = i;
        used[i] = reg;
        found = true;
        break;
      }
    }

    if (!found) {
      used[NUM_REGS - 1] = reg;
      int k = choose_to_spill(used);
      reg->real_reg = k;
      used[k]->real_reg = NUM_REGS - 1;
      used[k]->spill = true;
      used[k] = reg;
      error("error: register allocation %s\n", show_ireg(reg));
    }
  }
}

void alloc_regs(IProgram* prog) {
  for (int i = 0; i < prog->ifuncs->length; i++) {
    IFunc* func = prog->ifuncs->ptr[i];
    if (func->blocks) {
      Vector* regs = collect_regs(func);
      scan(regs);

      for (int i = 0; i < regs->length; i++) {
        eprintf("%s ", show_ireg(regs->ptr[i]));
      }
      eprintf("\n");
    } 

    // TODO: Reserve stack area to spilled register

    // TODO: Convert accesses to spilled registers to load and stores
  }
}
