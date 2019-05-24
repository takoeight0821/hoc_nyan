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

static Node* expr();
static Node* term();
static Node* integer();
static Node* add();
static Node* mul();
static Node* variable();
static Node* unary();
static Node* equality();
static Node* relational();
static Node* statement();

static Node* term() {
  if (match(TLPAREN)) {
    Node* node = add();
    if (!match(TRPAREN)) {
      parse_error("[RPAREN]", lt(0));
    }
    return node;
  } else if (la(0) == TIDENT && la(1) != TLPAREN) {
    return variable();
  } else if (la(0) == TIDENT) {
    Node* node = new_call_node(lt(0).ident, new_vec());
    consume();
    if (!match(TLPAREN)) {
      parse_error("(", lt(0));
    }
    if (match(TRPAREN)) {
      return node;
    }

    for (;;) {
      vec_push(node->args, expr());
      if (match(TRPAREN)) {
        return node;
      } else if (!match(TCOMMA)) {
        parse_error(",", lt(0));
      }
    }
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

static Node* equality() {
  Node* lhs = relational();
  for (;;) {
    if (match(TEQ)) {
      lhs = new_binop_node(NEQ, lhs, relational());
    } else if (match(TNE)) {
      lhs = new_binop_node(NNE, lhs, relational());
    } else {
      return lhs;
    }
  }
}

static Node* relational() {
  Node* lhs = add();
  for (;;) {
    if (match(TLT)) {
      lhs = new_binop_node(NLT, lhs, add());
    } else if (match(TLE)) {
      lhs = new_binop_node(NLE, lhs, add());
    } else if (match(TGT)) {
      lhs = new_binop_node(NGT, lhs, add());
    } else if (match(TGE)) {
      lhs = new_binop_node(NGE, lhs, add());
    } else {
      return lhs;
    }
  }
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

static Node* expr() {
  return equality();
}

static Node* statement() {
  Node* node;
  if (la(0) == TIDENT && streq(lt(0).ident, "return")) {
    consume();
    node = new_return_node(expr());
  } else if (la(0) == TIDENT && streq(lt(0).ident, "if")) {
    consume(); // if
    if (!match(TLPAREN)) {
      parse_error("(", lt(0));
    }
    Node* cond = expr();
    if (!match(TRPAREN)) {
      parse_error(")", lt(0));
    }
    Node* then = statement();

    if (la(0) == TIDENT && streq(lt(0).ident, "else")) {
      consume(); // else
      Node* els = statement();
      node = new_if_else_node(cond, then, els);
    } else {
      node = new_if_node(cond, then);
    }

    return node; // ; is not necessary

  } else if (la(0) == TIDENT && la(1) == TEQUAL) {
    Node* lhs = variable();
    consume();
    Node* rhs = expr();

    if (!map_has_key(vmap, lhs->name)) {
      lsize += 4; // sizeof(int)
      map_puti(vmap, lhs->name, lsize);
    }

    node = new_assign_node(lhs, rhs);

  } else if (match(TLBRACE)) {
    Vector* stmts = new_vec();
    while (la(0) != TRBRACE) {
      vec_push(stmts, statement());
    }
    Node* node = new_node(NBLOCK);
    node->stmts = stmts;

    if (!match(TRBRACE)) {
      parse_error("}", lt(0));
    }
    return node; // ; is not necessary
  } else {
    node = expr();
  };

  if (!match(TSEMICOLON)) {
    parse_error(";", lt(0));
  }

  return node;
}

Node* parse(Vector* tokens, Map *vars, size_t *local_size) {
  vmap = new_map();

  lookahead = calloc(tokens->length, sizeof(Token));
  for (size_t i = 0; i < tokens->length; i++) {
    lookahead[i] = *(Token*)(tokens->ptr[i]);
  }

  Node* ast = statement();

  if (!match(TEOF)) {
    parse_error("EOF", lt(0));
  }

  *vars = *vmap;
  *local_size = lsize;
  return ast;
}
