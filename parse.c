#include "hoc.h"
#include <stdbool.h>

Vector* tokens;
size_t current;
Node* node;

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

bool consume(enum TokenTag tag) {
  if (tag == peek()->tag) {
    next();
    return true;
  }
  return false;
}

void except(enum TokenTag tag) {
  if (!consume(tag)) {
    error("expect: unexpected token");
  }
}

Node* integer() {
  if (TINT == peek()->tag) {
    int i = peek()->integer;
    except(TINT);
    return new_int_node(i);
  }
  error("bad token");
}

Node* add() {
  Node* lhs = integer();
  for (;;) {
    if (consume(TPLUS)) {
      lhs = new_plus_node(lhs, integer());
    } else {
      return lhs;
    }
  }
}

Node* parse(Vector* tokens) {
  setup_parser(tokens);
  return add();
}
