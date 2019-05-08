#include "hoc.h"

static Token* lookahead;
static size_t p = 0; // 次の字句のインデックス

static void consume() {
  p++;
}

static Token lt(size_t i) {
  return lookahead[p + i];
}

static enum TokenTag la(size_t i) {
  return lt(i).tag;
}

static void match(enum TokenTag tag) {
  if (la(0) == tag) consume();
  else error("match error\n");
}

static Node* integer() {
  Node* n = new_int_node(lt(0).integer);
  match(TINT);
  return n;
}

static Node* add() {
  Node* lhs = integer();
  for (;;) {
    if (la(0) == TPLUS) {
      consume();
      lhs = new_plus_node(lhs, integer());
    } else {
      return lhs;
    }
  }
}

Node* parse(Vector* tokens) {
  lookahead = calloc(tokens->length, sizeof(Token));
  for (size_t i = 0; i < tokens->length; i++) {
    lookahead[i] = *(Token*)vec_get(tokens, i);
  }

  return add();
}
