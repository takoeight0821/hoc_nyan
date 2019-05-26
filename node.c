#include "hoc.h"

Node* new_node(enum NodeTag tag) {
  Node* node = malloc(sizeof(Node));
  node->tag = tag;
  return node;
}

/* void indent(int level) { */
/*   for (int i = 0; i < level; i++) { */
/*     printf(" "); */
/*   } */
/* } */

void dump_node(Node* node, int level) {
  /* indent(level); */
  assert(node);

  switch (node->tag) {
  case NINT:
    eprintf("%d", node->integer);
    break;
  case NVAR:
    eprintf("%s", node->name);
    break;
  case NCALL: {
    eprintf("(call ");
    eprintf("%s", node->name);
    for (size_t i = 0; i < node->args->length; i++) {
      eprintf(" ");
      dump_node(node->args->ptr[i], level+1);
    }
    eprintf(")");
    break;
  }
  case NASSIGN:
    eprintf("(= ");
    dump_node(node->lhs, level+1); eprintf(" ");
    dump_node(node->rhs, level+1);
    eprintf(")");
    break;
  case NPLUS:
    eprintf("(+ ");
    dump_node(node->lhs, level+1); eprintf(" ");
    dump_node(node->rhs, level+1);
    eprintf(")");
    break;
  case NMINUS:
    eprintf("(- ");
    dump_node(node->lhs, level+1); eprintf(" ");
    dump_node(node->rhs, level+1);
    eprintf(")");
    break;
  case NMUL:
    eprintf("(* ");
    dump_node(node->lhs, level+1); eprintf(" ");
    dump_node(node->rhs, level+1);
    eprintf(")");
    break;
  case NDIV:
    eprintf("(/ ");
    dump_node(node->lhs, level+1); eprintf(" ");
    dump_node(node->rhs, level+1);
    eprintf(")");
    break;
  case NLT:
    eprintf("(< ");
    dump_node(node->lhs, level+1); eprintf(" ");
    dump_node(node->rhs, level+1);
    eprintf(")");
    break;
  case NLE:
    eprintf("(<= ");
    dump_node(node->lhs, level+1); eprintf(" ");
    dump_node(node->rhs, level+1);
    eprintf(")");
    break;
  case NGT:
    eprintf("(> ");
    dump_node(node->lhs, level+1); eprintf(" ");
    dump_node(node->rhs, level+1);
    eprintf(")");
    break;
  case NGE:
    eprintf("(>= ");
    dump_node(node->lhs, level+1); eprintf(" ");
    dump_node(node->rhs, level+1);
    eprintf(")");
    break;
  case NEQ:
    eprintf("(== ");
    dump_node(node->lhs, level+1); eprintf(" ");
    dump_node(node->rhs, level+1);
    eprintf(")");
    break;
  case NNE:
    eprintf("(!= ");
    dump_node(node->lhs, level+1); eprintf(" ");
    dump_node(node->rhs, level+1);
    eprintf(")");
    break;
  case NRETURN:
    eprintf("(return ");
    dump_node(node->ret, level+1);
    eprintf(")");
    break;
  case NIF:
    eprintf("(if ");
    dump_node(node->cond, level+1);
    eprintf(" ");
    dump_node(node->then, level+1);
    eprintf(")");
    break;
  case NIFELSE:
    eprintf("(if ");
    dump_node(node->cond, level+1);
    eprintf(" ");
    dump_node(node->then, level+1);
    eprintf(" ");
    dump_node(node->els, level+1);
    eprintf(")");
    break;
  case NBLOCK: {
    eprintf("{\n");
    for (size_t i = 0; i < node->stmts->length; i++) {
      dump_node(node->stmts->ptr[i], level+1);
      puts("");
    }
    eprintf("}");
    break;
  }
  case NFUNCDEF: {
    eprintf("%s", node->name);
    eprintf("(");
    for (size_t i = 0; i < node->params->length; i++) {
      if (i != 0)
        eprintf(", ");
      eprintf("%s", node->params->ptr[i]);
    }
    eprintf(") ");
    dump_node(node->body, level);
    break;
  }
  }
}
