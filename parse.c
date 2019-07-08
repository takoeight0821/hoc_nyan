#include "hoc.h"

static Vector* tokens;
static size_t p = 0; // 次の字句のインデックス
static Map* local_env; // Map(char*, Var*)
static size_t local_size = 0;

static Var* new_var(char* name, Type* type, int offset) {
  Var* var = calloc(1, sizeof(Var));
  var->name = strdup(name);
  var->type = type;
  var->offset = offset;
  return var;
}

static Var* find_var(char* name) {
  return map_get(local_env, name);
}

static void consume() {
  p++;
}

static Token* lt(size_t i) {
  return tokens->ptr[p + i];
}

static enum TokenTag la(size_t i) {
  return lt(i)->tag;
}

static void parse_error(const char* expected, Token* actual) {
  bad_token(actual, format(" %s expected", expected));
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
  if (la(0) == TIDENT && streq(lt(0)->ident, name)) {
    consume();
    return 1;
  } else {
    return 0;
  }
}

static Node* variable(Token* t);
static Node* expr();
static Node* term();
static Node* integer();
static Node* add();
static Node* mul();
static Node* unary();
static Node* equality();
static Node* relational();
static Node* statement();

static Type* type_specifier() {
  Type* ty = calloc(1, sizeof(Type));

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
    Node* node = variable(lt(0));
    consume();
    return node;
  } else if (la(0) == TIDENT) {
    Node* node = new_node(NCALL);
    node->name = strdup(lt(0)->ident);
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
  if (match_ident("sizeof")) {
    Node* node = new_node(NSIZEOF);
    node->expr = unary();
    return node;
  } else if (match(TPLUS)) {
    return term();
  } else if (match(TMINUS)) {
    Node* node = new_node(NMINUS);
    Node* zero = new_node(NINT);
    zero->integer = 0;
    node->lhs = zero;
    node->rhs = term();
    return node;
  } else if (match(TAND)) {
    Node* node = new_node(NADDR);
    node->expr = term();
    return node;
  } else if (match(TASTERISK)) {
    Node* node = new_node(NDEREF);
    node->expr = term();
    return node;
  } else {
    return term();
  }
}

static Node* variable(Token* t) {
  assert(t->tag == TIDENT);
  Node* node = new_node(NVAR);
  node->var = find_var(t->ident);
  return node;
}

static Node* integer() {
  Node* node = new_node(NINT);
  node->integer = lt(0)->integer;
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

static int is_typename(Token* t) {
  if (t->tag == TIDENT) {
    return streq(t->ident, "int");
  } else {
    return 0;
  }
}

static Type* ptr_to(Type* ty) {
  Type* new_ty = calloc(1, sizeof(Type));
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
  Var* var = new_var(name, ty, local_size);
  map_put(local_env, name, var);
}

static Node* direct_decl(Type* ty) {
  if (la(0) != TIDENT) {
    parse_error("ident", lt(0));
  }
  Node* node = new_node(NDEFVAR);
  node->name = strdup(lt(0)->ident);
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
  Node* node = new_node(NEXPR_STMT);
  node->expr = expr();
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
    node->expr = expr();
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

Function* funcdef() {
  Type* ret_type = type_specifier();

  while (match(TASTERISK)) {
    ret_type = ptr_to(ret_type);
  }

  char* name = strdup(lt(0)->ident);

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
    if (is_typename(lt(0))) {
      Type* ty = type_specifier();
      Node* param_decl = declarator(ty); // NDEFVAR
      Var* param = find_var(param_decl->name);

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

  if (match(TSEMICOLON)) {
    Function* func = calloc(1, sizeof(Function));
    func->name = name;
    func->ret_type = ret_type;
    func->body = NULL;
    func->params = params;
    func->local_size = local_size;
    return func;
  }

  Node* body = statement();

  Function* func = calloc(1, sizeof(Function));
  func->name = name;
  func->ret_type = ret_type;
  func->body = body;
  func->params = params;
  func->local_size = local_size;

  return func;
}

Program* parse(Vector* token_vec) {
  tokens = token_vec;

  local_env = new_map();

  Program* prog = calloc(1, sizeof(Program));
  prog->funcs = new_vec();

  while (!match(TEOF)) {
    vec_push(prog->funcs, funcdef());
  }

  return prog;
}
