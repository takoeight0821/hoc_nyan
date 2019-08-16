#include "hoc.h"

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
    return format("%d", ir->imm_int);
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
    return format("%s = load %zu %s", show_ireg(ir->r0), ir->size, show_ireg(ir->r1));
  case ISTORE:
    return format("store %zu %s <- %s", ir->size, show_ireg(ir->r0), show_ireg(ir->r1));
  case ICALL:
    return format("%s = call %s %s", show_ireg(ir->r0), ir->func_name, show_args(ir->args));
  case IBR:
    return format("br %s %s %s", show_ireg(ir->r0), ir->label0, ir->label1);
  case IJMP:
    return format("jmp %s", ir->label0);
  case IRET:
    return format("ret %s", show_ireg(ir->r0));
  }
}

char* show_block(Block* block) {
  StringBuilder* sb = new_sb();
  sb_puts(sb, format("%s: {\n", block->label));
  for (int i = 0; i < block->instrs->length; i++) {
    sb_puts(sb, " ");
    sb_puts(sb, block->instrs->ptr[i]);
    sb_puts(sb, "\n");
  }
  sb_puts(sb, "}\n");
  return sb_run(sb);
}

char* show_ifunc(IFunc* ifunc) {
  StringBuilder* sb = new_sb();
  sb_puts(sb, format("=== %s(%s) ===\n", ifunc->name, ifunc->entry_label));
  for (int i = 0; i < ifunc->blocks->length; i++) {
    show_block(ifunc->blocks->ptr[i]);
  }
  return sb_run(sb);
}
