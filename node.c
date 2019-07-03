#include "hoc.h"

Node* new_node(enum NodeTag tag) {
  Node* node = calloc(1, sizeof(Node));
  node->tag = tag;
  return node;
}

size_t size_of(Type* ty) {
  switch (ty->ty) {
  case TY_INT:
    return 8;
  case TY_PTR:
    return 8;
  }
}

Type* new_type() {
  Type* ty = calloc(1, sizeof(Type));
  return ty;
}

Type* type_of(Node* node) {
  if (node->type) {
    return node->type;
  } else if (node->tag == NVAR) {
    node->type = node->var->type;
    return node->type;
  } else {
    error("node must be type checked");
  }
}

void dump_type(Type* ty) {
  eprintf("%s", show_type(ty));
}

char* show_type(Type* ty) {
  assert(ty);

  switch (ty->ty) {
  case TY_INT:
    return format("int");
  case TY_PTR:
    return format("ptr(%s)", show_type(ty->ptr_to));
  }
}

static char* show_indent(int level) {
  return format("%*s", level, "");
}

void indent(int level) {
  eprintf("%s", show_indent(level));
}

void dump_node(Node* node, int level) {
  assert(node);

  switch (node->tag) {
  case NINT:
    indent(level);
    eprintf("%d\n", node->integer);
    break;
  case NVAR:
    indent(level);
    eprintf("%s : ", node->var->name);
    dump_type(node->var->type);
    eprintf("\n");
    break;
  case NPLUS:
    indent(level);
    eprintf("(+\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NMINUS:
    indent(level);
    eprintf("(-\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NMUL:
    indent(level);
    eprintf("(*\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NDIV:
    indent(level);
    eprintf("(/\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NLT:
    indent(level);
    eprintf("(<\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NLE:
    indent(level);
    eprintf("(<=\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NGT:
    indent(level);
    eprintf("(>\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NGE:
    indent(level);
    eprintf("(>=\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NEQ:
    indent(level);
    eprintf("(==\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NNE:
    indent(level);
    eprintf("(!=\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NDEFVAR: {
    indent(level);
    eprintf("(");
    dump_type(node->type);
    eprintf(" %s)\n", node->name);
    break;
  }
  case NASSIGN:
    indent(level);
    eprintf("(=\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NCALL: {
    indent(level);
    eprintf("(call\n");
    indent(level + 1);
    eprintf("%s\n", node->name);
    for (size_t i = 0; i < node->args->length; i++) {
      dump_node(node->args->ptr[i], level+1);
    }
    indent(level);
    eprintf(")\n");
    break;
  }
  case NADDR:
    indent(level);
    eprintf("(&\n");
    dump_node(node->expr, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NDEREF:
    indent(level);
    eprintf("(*\n");
    dump_node(node->expr, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NEXPR_STMT:
    indent(level);
    eprintf("(expr_stmt\n");
    dump_node(node->expr, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NRETURN:
    indent(level);
    eprintf("(return\n");
    dump_node(node->expr, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NIF:
    indent(level);
    eprintf("(if\n");
    dump_node(node->cond, level+1);
    dump_node(node->then, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NIFELSE:
    indent(level);
    eprintf("(if\n");
    dump_node(node->cond, level+1);
    dump_node(node->then, level+1);
    dump_node(node->els, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NWHILE:
    indent(level);
    eprintf("(while\n");
    dump_node(node->cond, level+1);
    dump_node(node->body, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NFOR:
    indent(level);
    eprintf("(for\n");
    dump_node(node->init, level+1);
    dump_node(node->cond, level+1);
    dump_node(node->step, level+1);
    dump_node(node->body, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NBLOCK: {
    indent(level);
    eprintf("{\n");
    for (size_t i = 0; i < node->stmts->length; i++) {
      dump_node(node->stmts->ptr[i], level+1);
    }
    indent(level);
    eprintf("}\n");
    break;
  }
  }
}

void dump_function(Function* func) {
  eprintf("%s ", func->name);
  eprintf("(");
  for (size_t i = 0; i < func->params->length; i++) {
    if (i != 0)
      eprintf(", ");
    dump_type(((Var*)func->params->ptr[i])->type);
    eprintf(" %s", ((Var*)func->params->ptr[i])->name);
  }
  eprintf(")\n");
  dump_node(func->body, 1);
}
