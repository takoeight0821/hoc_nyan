#include "hoc.h"

Node* new_node(enum NodeTag tag) {
  Node* node = malloc(sizeof(Node));
  node->tag = tag;
  return node;
}

void indent(int level) {
  for (int i = 0; i < level; i++) {
    eprintf("  ");
  }
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
    eprintf("%s\n", node->name);
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
  case NDEFVAR: {
    indent(level);
    eprintf("(int %s)\n", node->name);
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
  case NRETURN:
    indent(level);
    eprintf("(return\n");
    dump_node(node->ret, level+1);
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
  case NFUNCDEF: {
    eprintf("%s ", node->name);
    eprintf("(");
    for (size_t i = 0; i < node->params->length; i++) {
      if (i != 0)
        eprintf(", ");
      eprintf("%s", ((Node*)node->params->ptr[i])->name);
    }
    eprintf(")\n");
    dump_node(node->body, level+1);
    break;
  }
  }
}
