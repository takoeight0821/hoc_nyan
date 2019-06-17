#include "hoc.h"

static Token* lookahead;
static size_t p = 0; // 次の字句のインデックス
static Map* local_env;
static size_t local_size = 0;
static Map* local_type_env;

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

static int match_ident(char* name) {
  if (la(0) == TIDENT && streq(lt(0).ident, name)) {
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

static Type* type_specifier() {
  Type* ty = malloc(sizeof(Type));

  enum {
    INT = 1,
  };

  int base_type = 0;

  for (;;) {
    if (match_ident("int")) {
      base_type += INT;
    } else {
      break;
    }
  }

  switch (base_type) {
  case INT:
    *ty = (Type){TY_INT, NULL};
    break;
  default:
    return NULL;
  }

  return ty;
}

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
  node->type = map_get(local_type_env, node->name);
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

static int is_typename(Token t) {
  if (t.tag == TIDENT) {
    return streq(t.ident, "int");
  } else {
    return 0;
  }
}

static Type* ptr_to(Type* ty) {
  Type* new_ty = malloc(sizeof(Type));
  new_ty->ty = TY_PTR;
  new_ty->ptr_to = ty;
  return new_ty;
}

// TODO: 名前解決
static void add_lvar(char* name, Type* ty) {
  if (map_has_key(local_env, name)) {
    error("%s is already defined\n", name);
  }

  local_size += size_of(ty);
  map_puti(local_env, name, local_size);
  map_put(local_type_env, name, ty);
}

static Node* direct_decl(Type* ty) {
  if (la(0) != TIDENT) {
    parse_error("ident", lt(0));
  }
  Node* node = new_node(NDEFVAR);
  node->name = strdup(lt(0).ident);
  node->type = ty;
  consume();

  add_lvar(node->name, ty);

  return node;
}

static Node* declarator(Type* ty) {
  while (match(TASTERISK)) {
    ty = ptr_to(ty);
  }
  return direct_decl(ty);
}

static Node* declaration() {
  // variable definition
  Type* ty = type_specifier();

  Node* decl = declarator(ty);

  if (!match(TSEMICOLON)) {
    parse_error(";", lt(0));
  }

  return decl;
};

static Node* expr_stmt() {
  Node* node = expr();
  if (!match(TSEMICOLON)) {
    parse_error(";", lt(0));
  }
  return node;
};

static Node* statement() {
  if (is_typename(lt(0))) {
    return declaration();
  } else if (match_ident("return")) {
    Node* node = new_node(NRETURN);
    node->ret = expr();
    if (!match(TSEMICOLON)) {
      parse_error(";", lt(0));
    }
    return node;
  } else if (match_ident("if")) {
    if (!match(TLPAREN)) {
      parse_error("(", lt(0));
    }
    Node* cond = expr();
    if (!match(TRPAREN)) {
      parse_error(")", lt(0));
    }

    Node* then = statement();

    if (match_ident("else")) {
      Node* els = statement();
      Node* node = new_node(NIFELSE);
      node->cond = cond;
      node->then = then;
      node->els = els;
      return node;
    } else {
      Node* node = new_node(NIF);
      node->cond = cond;
      node->then = then;
      return node;
    }
  } else if (match_ident("while")) {
    Node* node = new_node(NWHILE);

    if (!match(TLPAREN)) {
      parse_error("(", lt(0));
    }
    node->cond = expr();
    if (!match(TRPAREN)) {
      parse_error(")", lt(0));
    }

    node->body = statement();

    return node;
  } else if (match_ident("for")) {
    Node* node = new_node(NFOR);

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

    return node;
  } else if (match(TLBRACE)) {
    Node* node = new_node(NBLOCK);
    node->stmts = new_vec();
    while (la(0) != TRBRACE) {
      vec_push(node->stmts, statement());
    }

    if (!match(TRBRACE)) {
      parse_error("}", lt(0));
    }
    return node;
  } else {
    return expr_stmt();
  }
}

Node* funcdef() {
  if (!match_ident("int")) {
    parse_error("int", lt(0));
  }

  char* name = strdup(lt(0).ident);

  if (!match(TIDENT)) {
    parse_error("function name", lt(0));
  }
  if (!match(TLPAREN)) {
    parse_error("(", lt(0));
  }

  Vector* params = new_vec();
  local_env = new_map();
  local_type_env = new_map();
  local_size = 0;

  for (;;) {
    if (is_typename(lt(0))) {
      Type* ty = type_specifier();
      Node* param_decl = declarator(ty); // NDEFVAR
      Node* param = new_node(NVAR);
      param->name = param_decl->name;
      param->offset = map_geti(local_env, param->name);
      param->type = map_get(local_type_env, param->name);

      vec_push(params, param);
      if (match(TCOMMA))
        continue;
      if (match(TRPAREN))
        break;
      parse_error(", or )", lt(0));

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
  local_type_env = new_map();

  Vector* funcdefs = new_vec();
  while (!match(TEOF)) {
    vec_push(funcdefs, funcdef());
  }

  return funcdefs;
}
