#include "hoc.h"

static Token* lookahead;
static size_t p = 0; // 次の字句のインデックス
static Map* local_env;
static size_t local_size = 0;

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
    Node* node = new_node(NCALL);
    node->name = strdup(lt(0).ident);
    node->args = new_vec();
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
    Node* node = new_node(NMINUS);
    Node* zero = new_node(NINT);
    zero->integer = 0;
    node->lhs = zero;
    node->rhs = term();
    return node;
  } else {
    return term();
  }
}

static Node* variable() {
  Node* node = new_node(NVAR);
  node->name = strdup(lt(0).ident);
  node->offset = map_geti(local_env, node->name);
  if (!match(TIDENT)) {
    parse_error("variable", lt(0));
  }
  return node;
}

static Node* integer() {
  Node* node = new_node(NINT);
  node->integer = lt(0).integer;
  if (!match(TINT)) {
    parse_error("integer", lt(0));
  }
  return node;
}

static Node* equality() {
  Node* lhs = relational();
  for (;;) {
    if (match(TEQ)) {
      Node* node = new_node(NEQ);
      node->lhs = lhs;
      node->rhs = relational();
      lhs = node;
    } else if (match(TNE)) {
      Node* node = new_node(NNE);
      node->lhs = lhs;
      node->rhs = relational();
      lhs = node;
    } else {
      return lhs;
    }
  }
}

static Node* relational() {
  Node* lhs = add();
  for (;;) {
    if (match(TLT)) {
      Node* node = new_node(NLT);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if (match(TLE)) {
      Node* node = new_node(NLE);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if (match(TGT)) {
      Node* node = new_node(NGT);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if (match(TGE)) {
      Node* node = new_node(NGE);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else {
      return lhs;
    }
  }
}
static Node* mul() {
  Node* lhs = unary();
  for (;;) {
    if (match(TASTERISK)) {
      Node* node = new_node(NMUL);
      node->lhs = lhs;
      node->rhs = unary();
      lhs = node;
    } else if (match(TSLASH)) {
      Node* node = new_node(NDIV);
      node->lhs = lhs;
      node->rhs = unary();
      lhs = node;
    } else {
      return lhs;
    }
  }
}

static Node* add() {
  Node* lhs = mul();
  for (;;) {
    if (match(TPLUS)) {
      Node* node = new_node(NPLUS);
      node->lhs = lhs;
      node->rhs = mul();
      lhs = node;
    } else if (match(TMINUS)) {
      Node* node = new_node(NMINUS);
      node->lhs = lhs;
      node->rhs = mul();
      lhs = node;
    } else {
      return lhs;
    }
  }
}

static Node* assign() {
  if (la(0) == TIDENT) {
    if (la(1) == TEQUAL) {
      Node* lhs = equality();
      consume(); // =
      Node* rhs = assign();

      if (!map_has_key(local_env, lhs->name)) {
        error("%s is not defined\n", lhs->name);
      }

      Node* node = new_node(NASSIGN);
      node->lhs = lhs;
      node->rhs = rhs;
      return node;
    }
  }

  return equality();
}

static Node* expr() {
  return assign();
}

static Node* statement() {
  Node* node;
  if (la(0) == TIDENT && streq(lt(0).ident, "int")) {
    // variable definition
    consume(); // int
    node = new_node(NDEFVAR);

    if (la(0) != TIDENT) {
      parse_error("ident", lt(0));
    }
    node->name = strdup(lt(0).ident);
    consume(); // ident

    if (map_has_key(local_env, node->name)) {
      error("%s is already defined\n", node->name);
    }

    local_size += 4; // sizeof(int)
    map_puti(local_env, node->name, local_size);

    if (!match(TSEMICOLON)) {
      parse_error(";", lt(0));
    }

    return node;
  } else if (la(0) == TIDENT && streq(lt(0).ident, "return")) {
    consume();
    node = new_node(NRETURN);
    node->ret = expr();
    if (!match(TSEMICOLON)) {
      parse_error(";", lt(0));
    }
    return node;
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
      node = new_node(NIFELSE);
      node->cond = cond;
      node->then = then;
      node->els = els;
    } else {
      node = new_node(NIF);
      node->cond = cond;
      node->then = then;
    }

    return node; // ; is not necessary
  } else if (la(0) == TIDENT && streq(lt(0).ident, "while")) {
    consume(); // while
    if (!match(TLPAREN)) {
      parse_error("(", lt(0));
    }
    Node* cond = expr();
    if (!match(TRPAREN)) {
      parse_error(")", lt(0));
    }
    Node* body = statement();

    node = new_node(NWHILE);
    node->cond = cond;
    node->body = body;

    return node; // ; is not necessary
  } else if (la(0) == TIDENT && streq(lt(0).ident, "for")) {
    consume(); // for
    node = new_node(NFOR);

    if (!match(TLPAREN)) {
      parse_error("(", lt(0));
    }
    node->init = expr();
    if (!match(TSEMICOLON)) {
      parse_error(";", lt(0));
    }
    node->cond = expr();
    if (!match(TSEMICOLON)) {
      parse_error(";", lt(0));
    }
    node->step = expr();
    if(!match(TRPAREN)) {
      parse_error(")", lt(0));
    }
    node->body = statement();

    return node; // ; is not necessary
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
    if (!match(TSEMICOLON)) {
      parse_error(";", lt(0));
    }
    return node;
  };

}

Node* funcdef() {
  if (!(la(0) == TIDENT && streq(lt(0).ident, "int"))) {
    parse_error("int", lt(0));
  }
  consume();

  char* name = strdup(lt(0).ident);

  if (!match(TIDENT)) {
    parse_error("function name", lt(0));
  }
  if (!match(TLPAREN)) {
    parse_error("(", lt(0));
  }

  Vector* params = new_vec();
  local_env = new_map();
  local_size = 0;

  for (;;) {
    if (la(0) == TIDENT && streq(lt(0).ident, "int")) {
      consume();
      if (la(0) == TIDENT) {
        char* param = strdup(lt(0).ident);

        local_size += 4; // sizeof(int);
        map_puti(local_env, param, local_size);

        vec_push(params, variable());
        if (match(TCOMMA))
          continue;
        if (match(TRPAREN))
          break;
        parse_error(", or )", lt(0));
      }
    } else if (match(TRPAREN)) {
      break;
    } else {
      parse_error(")", lt(0));
    }
  }

  Node* body = statement();

  Node* node = new_node(NFUNCDEF);
  node->name = name;
  node->params = params;
  node->body = body;
  node->local_size = local_size;

  return node;
}

Vector* parse(Vector* tokens) {
  lookahead = calloc(tokens->length, sizeof(Token));
  for (size_t i = 0; i < tokens->length; i++) {
    lookahead[i] = *(Token*)(tokens->ptr[i]);
  }

  local_env = new_map();

  Vector* funcdefs = new_vec();
  while (!match(TEOF)) {
    vec_push(funcdefs, funcdef());
  }

  return funcdefs;
}
