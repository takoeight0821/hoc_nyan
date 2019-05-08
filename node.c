#include "hoc.h"
#include <stdio.h>
#include <stdlib.h>

Node* new_node(enum NodeTag tag) {
  Node* node = malloc(sizeof(Node));
  node->tag = tag;
  return node;
}

Node* new_plus_node(Node* lhs, Node* rhs) {
  Node* node = new_node(NPLUS);

  node->lhs = lhs;
  node->rhs = rhs;

  return node;
}

Node* new_int_node(int integer) {
  Node* node = new_node(NINT);

  node->integer = integer;

  return node;
}

void indent(int level) {
  for (int i = 0; i < level; i++) {
    printf(" ");
  }
}

void dump_node(Node* node, int level) {
  indent(level);

  if (node->tag == NINT) {
    printf("%d", node->integer);
  } else if (node->tag == NPLUS) {
    printf("+:\n");
    dump_node(node->lhs, level+1); puts("");
    fflush(stdout);
    dump_node(node->rhs, level+1); puts("");
  }
}
