#include "hoc.h"
#include <stdio.h>
#include <stdlib.h>

Node* new_node(enum NodeTag tag) {
  Node* node = malloc(sizeof(Node));
  node->tag = tag;
  return node;
}

Node* new_binop_node(enum NodeTag tag, Node* lhs, Node* rhs) {
  Node* node = new_node(tag);

  node->lhs = lhs;
  node->rhs = rhs;

  return node;
}

Node* new_int_node(int integer) {
  Node* node = new_node(NINT);

  node->integer = integer;

  return node;
}

Node* new_return_node(Node* ret) {
  Node* node = new_node(NRETURN);

  node->ret = ret;

  return node;
}

Node* new_stmts_node(Vector* stmts) {
  Node* node = new_node(NSTMTS);

  node->stmts = stmts;

  return node;
}

/* void indent(int level) { */
/*   for (int i = 0; i < level; i++) { */
/*     printf(" "); */
/*   } */
/* } */

void dump_node(Node* node, int level) {
  /* indent(level); */

  switch (node->tag) {
  case NINT:
    eprintf("%d", node->integer);
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
  case NRETURN:
    eprintf("(return ");
    dump_node(node->ret, level+1);
    eprintf(")");
    break;
  case NSTMTS: {
    for (size_t i = 0; i < node->stmts->length; i++) {
      dump_node(node->stmts->ptr[i], level);
      puts("");
    }
    break;
  }
  }
}
