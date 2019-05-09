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

static Node* term();
static Node* integer();
static Node* add();
static Node* mul();

static Node* term() {
  if (la(0) == TLPAREN) {
    match(TLPAREN);
    Node* node = add();
    if (la(0) != TRPAREN) {
      error("mismatch parens");
    }
    match(TRPAREN);
    return node;
  } else {
    return integer();
  }
}

static Node* integer() {
  Node* n = new_int_node(lt(0).integer);
  match(TINT);
  return n;
}

static Node* mul() {
  Node* lhs = term();
  for (;;) {
    if (la(0) == TASTERISK) {
      match(TASTERISK);
      lhs = new_binop_node(NMUL, lhs, term());
    } else {
      return lhs;
    }
  }
}

static Node* add() {
  Node* lhs = mul();
  for (;;) {
    if (la(0) == TPLUS) {
      match(TPLUS);
      lhs = new_binop_node(NPLUS, lhs, mul());
    } else if (la(0) == TMINUS) {
      match(TMINUS);
      lhs = new_binop_node(NMINUS, lhs, mul());
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
