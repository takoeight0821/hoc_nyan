#include "hoc.h"

static Token* lookahead;
static size_t p = 0; // 次の字句のインデックス
static Map* vmap;
static size_t lsize = 0;

static void consume() {
  p++;
}

static Token lt(size_t i) {
  return lookahead[p + i];
}

static enum TokenTag la(size_t i) {
  return lt(i).tag;
}

static void parse_error(const char* expected, Token actual) {
  eprintf("%s expected, but got ", expected);
  dump_token(actual);
  eprintf("\n");
  exit(1);
}

static int match(enum TokenTag tag) {
  if (la(0) == tag) {
    consume();
    return 1;
  } else {
    return 0;
  }
}

static Node* term();
static Node* integer();
static Node* add();
static Node* mul();
static Node* variable();
static Node* unary();

static Node* term() {
  if (match(TLPAREN)) {
    Node* node = add();
    if (!match(TRPAREN)) {
      parse_error("[RPAREN]", lt(0));
    }
    return node;
  } else if (la(0) == TIDENT) {
    return variable();
  } else {
    return integer();
  }
}

static Node* unary() {
  if (match(TPLUS)) {
    return term();
  } else if (match(TMINUS)) {
    Node* node = new_binop_node(NMINUS, new_int_node(0), term());
    return node;
  } else {
    return term();
  }
}

static Node* variable() {
  Node* n = new_var_node(lt(0).ident);
  if (!match(TIDENT)) {
    parse_error("variable", lt(0));
  }
  return n;
}

static Node* integer() {
  Node* n = new_int_node(lt(0).integer);
  if (!match(TINT)) {
    parse_error("integer", lt(0));
  }
  return n;
}

static Node* mul() {
  Node* lhs = unary();
  for (;;) {
    if (match(TASTERISK)) {
      lhs = new_binop_node(NMUL, lhs, unary());
    } else if (match(TSLASH)) {
      lhs = new_binop_node(NDIV, lhs, unary());
    } else {
      return lhs;
    }
  }
}

static Node* add() {
  Node* lhs = mul();
  for (;;) {
    if (match(TPLUS)) {
      lhs = new_binop_node(NPLUS, lhs, mul());
    } else if (match(TMINUS)) {
      lhs = new_binop_node(NMINUS, lhs, mul());
    } else {
      return lhs;
    }
  }
}

static Node* statement() {
  Node* node;
  if (la(0) == TIDENT && streq(lt(0).ident, "return")) {
    consume();
    node = new_return_node(add());
  } else if (la(0) == TIDENT && la(1) == TEQUAL) {
    Node* lhs = variable();
    consume();
    Node* rhs = add();

    if (!map_has_key(vmap, lhs->ident)) {
      lsize += 4; // sizeof(int)
      map_put(vmap, lhs->ident, (void*)lsize);
    }

    node = new_assign_node(lhs, rhs);

  } else {
    node = add();
  };

  if (!match(TSEMICOLON)) {
    parse_error(";", lt(0));
  }

  return node;
}

static Node* statements() {
  Vector* stmts = new_vec();
  while (la(0) != TEOF) {
    vec_push(stmts, statement());
  }
  Node* node = new_node(NSTMTS);
  node->stmts = stmts;
  return node;
}

Node* parse(Vector* tokens, Map *vars, size_t *local_size) {
  vmap = new_map();

  lookahead = calloc(tokens->length, sizeof(Token));
  for (size_t i = 0; i < tokens->length; i++) {
    lookahead[i] = *(Token*)(tokens->ptr[i]);
  }

  Node* ast = statements();
  *vars = *vmap;
  *local_size = lsize;
  return ast;
}
