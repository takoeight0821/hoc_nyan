#include <hoc.h>

/* create IR */

IReg* real_reg(Reg reg) {
  IReg* new = calloc(1, sizeof(IReg));
  new->id = reg;
  new->is_real = true;
  return new;
}

IR* new_ir(enum IRTag op) {
  IR* new = calloc(1, sizeof(IR));
  new->op = op;
  return new;
}

IR* new_binop_ir(enum IRTag op, IReg* r0, IReg* r1, IReg* r2) {
  IR* new = new_ir(op);
  new->r0 = r0;
  new->r1 = r1;
  new->r2 = r2;
  return new;
}

/* printing IR */
char* show_ireg(IReg* reg) {
  return format("$%d", reg->id);
}

static char* show_args(Vector* args) {
  StringBuilder* sb = new_sb();
  sb_puts(sb, "(");
  for (int i = 0; i < args->length; i++) {
    if (i) {
      sb_puts(sb, " ");
    }
    sb_puts(sb, show_ireg(args->ptr[i]));
  }
  sb_puts(sb, ")");
  return sb_run(sb);
}

char* show_ir(IR* ir) {
  switch (ir->op) {
  case IIMM:
    return format("%s = %d", show_ireg(ir->r0), ir->imm_int);
  case ILABEL:
    return format("%s = %s", show_ireg(ir->r0), ir->label);
  case IADD:
    return format("%s = %s + %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case ISUB:
    return format("%s = %s - %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case IMUL:
    return format("%s = %s * %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case IDIV:
    return format("%s = %s / %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case IMOD:
    return format("%s = %s %% %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case ILT:
    return format("%s = %s < %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case ILE:
    return format("%s = %s <= %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case IGT:
    return format("%s = %s > %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case IGE:
    return format("%s = %s >= %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case IEQ:
    return format("%s = %s == %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case INE:
    return format("%s = %s != %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case IAND:
    return format("%s = %s & %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case IOR:
    return format("%s = %s | %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case IXOR:
    return format("%s = %s ^ %s", show_ireg(ir->r0), show_ireg(ir->r1), show_ireg(ir->r2));
  case ILOAD:
    return format("%s = load %s", show_ireg(ir->r0), show_ireg(ir->r1));
  case ISTORE:
    return format("store %s <- %s", show_ireg(ir->r0), show_ireg(ir->r1));
  case IMOV:
    return format("mov %s <- %s", show_ireg(ir->r0), show_ireg(ir->r1));
  case ICALL:
    return format("%s = call %s %s", show_ireg(ir->r0), ir->func_name, show_args(ir->args));
  case IBR:
    return format("br %s %s %s", show_ireg(ir->r0), ir->then->label, ir->els->label);
  case IJMP:
    return format("jmp %s", ir->jump_to->label);
  case IRET:
    return format("ret %s", show_ireg(ir->r0));
  }
  error("unreachable\n");
}

char* show_block(Block* block) {
  StringBuilder* sb = new_sb();
  sb_puts(sb, format("%s: {\n", block->label));
  for (int i = 0; i < block->instrs->length; i++) {
    sb_puts(sb, " ");
    sb_puts(sb, show_ir(block->instrs->ptr[i]));
    sb_puts(sb, "\n");
  }
  sb_puts(sb, "}");
  return sb_run(sb);
}

char* show_ifunc(IFunc* ifunc) {
  StringBuilder* sb = new_sb();
  sb_puts(sb, format("=== %s(%s) ===\n", ifunc->name, ifunc->entry_label));
  for (int i = 0; i < ifunc->blocks->length; i++) {
    sb_puts(sb, show_block(ifunc->blocks->ptr[i]));
    sb_puts(sb, "\n");
  }
  return sb_run(sb);
}

char* show_iprog(IProgram* iprog) {
  StringBuilder* sb = new_sb();
  for (int i = 0; i < iprog->ifuncs->length; i++) {
    sb_puts(sb, show_ifunc(iprog->ifuncs->ptr[i]));
  }
  return sb_run(sb);
}
