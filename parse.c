#include "hoc.h"
#include <stdbool.h>

Vector* tokens;
size_t current;

void setup_parser(Vector* t) {
  tokens = t;
  current = 0;
}

void next() {
  current++;
}

Token* peek() {
  return (Token*)vec_get(tokens, current);
}

bool accept(enum TokenTag tag) {
  if (tag == peek()->tag) {
    next();
    return true;
  }
  return false;
}

void except(enum TokenTag tag) {
  if (!accept(tag)) {
    error("expect: unexpected token");
  }
}

Node* integer() {
  int i = peek()->integer;
  except(TINT);
  return new_int_node(i);
}

Node* add() {
  Node* lhs = integer();
  except(TPLUS);
  Node* rhs = integer();
  return new_plus_node(lhs, rhs);
}

Node* parse(Vector* tokens) {
  setup_parser(tokens);
  return add();
}
