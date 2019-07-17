#include "hoc.h"

struct env {
  Map* vars;
  struct env* prev;
};

struct env* new_env(struct env* prev) {
  struct env* env = calloc(1, sizeof(struct env));
  env->vars = new_map();
  env->prev = prev;
  return env;
}

static Vector* tokens;
static size_t p = 0; // 次の字句のインデックス
static struct env* local_env; // Map(char*, Var*)
static size_t local_size = 0;
static Map* global_env;
static Vector* strs;

static Var* new_var(char* name, Type* type, int offset) {
  Var* var = calloc(1, sizeof(Var));
  var->name = name;
  var->type = type;
  var->offset = offset;
  var->is_local = true;
  return var;
}

static Var* find_var(Token* tok, char* name) {
  Var* var = NULL;
  for (struct env* env = local_env; var == NULL && env != NULL; env = env->prev) {
    var = map_get(env->vars, name);
  }

  if (var == NULL) {
    var = map_get(global_env, name);
  }

  if (var == NULL) {
    bad_token(tok, format("%s is not defined\n", name));
  }
  return var;
}

static void add_lvar(Token* tok, char* name, Type* ty) {
  if (map_has_key(local_env->vars, name)) {
    bad_token(tok, format("%s is already defined\n", name));
  }

  local_size += size_of(ty);
  Var* var = new_var(name, ty, local_size);
  map_put(local_env->vars, name, var);
}

static Var* new_gvar(char* name, Type* type) {
  Var* gvar = calloc(1, sizeof(Var));
  gvar->name = name;
  gvar->type = type;
  gvar->is_local = false;
  return gvar;
}

static void add_gvar(char* name, Type* type) {
  if (map_has_key(global_env, name)) {
    error("%s is already defined\n", name);
  }
  map_put(global_env, name, new_gvar(name, type));
}

// 文字列リテラルのインターン
static size_t intern(char* str) {
  size_t offset = strs->length;
  vec_push(strs, str);
  return offset;
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
  bad_token(actual, format("%s expected", expected));
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
static Node* string();
static Node* add();
static Node* mul();
static Node* unary();
static Node* equality();
static Node* relational();
static Node* statement();

static Type* type_specifier() {
  Type* ty = calloc(1, sizeof(Type));

  if (match_ident("int")) {
    ty->ty = TY_INT;
  } else if (match_ident("char")) {
    ty->ty = TY_CHAR;
  } else {
    ty = NULL;
  }

  return ty;
}

static Node* term() {
  if (match(TLPAREN)) {
    Node* node = add();
    if (!match(TRPAREN)) {
      parse_error(")", lt(0));
    }
    return node;
  } else if (la(0) == TIDENT && la(1) != TLPAREN) {
    Node* node = variable(lt(0));
    consume();
    return node;
  } else if (la(0) == TIDENT) {
    Node* node = new_node(NCALL, lt(0));
    node->name = lt(0)->ident;
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
  } else if (la(0) == TSTRING) {
    return string();
  } else {
    return integer();
  }
}

static Node* unary() {
  if (match_ident("sizeof")) {
    Node* node = new_node(NSIZEOF, tokens->ptr[p - 1]);
    node->expr = unary();
    return node;
  } else if (match(TPLUS)) {
    return term();
  } else if (match(TMINUS)) {
    Node* node = new_node(NMINUS, tokens->ptr[p - 1]);
    Node* zero = new_node(NINT, tokens->ptr[p - 1]);
    zero->integer = 0;
    node->lhs = zero;
    node->rhs = term();
    return node;
  } else if (match(TAND)) {
    Node* node = new_node(NADDR, tokens->ptr[p - 1]);
    node->expr = term();
    return node;
  } else if (match(TASTERISK)) {
    Node* node = new_node(NDEREF, tokens->ptr[p - 1]);
    node->expr = term();
    return node;
  } else {
    Node* node = term();

    if (match(TLBRACK)) {
      Token* t = tokens->ptr[p - 1];
      Node* offset = expr();
      if (!match(TRBRACK)) {
        parse_error("]", lt(0));
      }
      Node* deref = new_node(NDEREF, t);
      deref->expr = new_node(NPLUS, t);
      deref->expr->lhs = node;
      deref->expr->rhs = offset;
      node = deref;
    }

    return node;
  }
}

static Node* variable(Token* t) {
  assert(t->tag == TIDENT);
  Node* node = new_node(NVAR, t);
  node->var = find_var(t, t->ident);

  return node;
}

static Node* integer() {
  Node* node = new_node(NINT, lt(0));
  node->integer = lt(0)->integer;
  if (!match(TINT)) {
    parse_error("integer", lt(0));
  }
  return node;
}

static Node* string() {
  Node* node = new_node(NSTRING, lt(0));
  char* str = lt(0)->str;

  if (!match(TSTRING)) {
    parse_error("string", lt(0));
  }

  node->str_id = intern(str);
  return node;
}

static Node* equality() {
  Node* lhs = relational();
  for (;;) {
    if (match(TEQ)) {
      Node* node = new_node(NEQ, tokens->ptr[p - 1]);
      node->lhs = lhs;
      node->rhs = relational();
      lhs = node;
    } else if (match(TNE)) {
      Node* node = new_node(NNE, tokens->ptr[p - 1]);
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
      Node* node = new_node(NLT, tokens->ptr[p - 1]);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if (match(TLE)) {
      Node* node = new_node(NLE, tokens->ptr[p - 1]);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if (match(TGT)) {
      Node* node = new_node(NGT, tokens->ptr[p - 1]);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if (match(TGE)) {
      Node* node = new_node(NGE, tokens->ptr[p - 1]);
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
      Node* node = new_node(NMUL, tokens->ptr[p - 1]);
      node->lhs = lhs;
      node->rhs = unary();
      lhs = node;
    } else if (match(TSLASH)) {
      Node* node = new_node(NDIV, tokens->ptr[p - 1]);
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
      Node* node = new_node(NPLUS, tokens->ptr[p - 1]);
      node->lhs = lhs;
      node->rhs = mul();
      lhs = node;
    } else if (match(TMINUS)) {
      Node* node = new_node(NMINUS, tokens->ptr[p - 1]);
      node->lhs = lhs;
      node->rhs = mul();
      lhs = node;
    } else {
      return lhs;
    }
  }
}

static Node* assign() {
  Node* node = equality();

  if (match(TEQUAL)) {
    Node* lhs = node;
    Node* rhs = assign();
    node = new_node(NASSIGN, lhs->token);
    node->lhs = lhs;
    node->rhs = rhs;
  }

  return node;
}

static Node* expr() {
  return assign();
}

static int is_typename(Token* t) {
  if (t->tag == TIDENT) {
    return streq(t->ident, "int") || streq(t->ident, "char");
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

static Node* direct_decl(Type* ty) {
  if (la(0) != TIDENT) {
    parse_error("ident", lt(0));
  }
  Node* node = new_node(NDEFVAR, lt(0));
  node->name = lt(0)->ident;
  consume();

  if (match(TLBRACK)) {
    Type* array_ty = new_type();
    array_ty->ty = TY_PTR;
    array_ty->ptr_to = ty;
    array_ty->array_size = lt(0)->integer;
    ty = array_ty;
    if (!match(TINT)) {
      parse_error("integer", lt(0));
    }
    if (!match(TRBRACK)) {
      parse_error("]", lt(0));
    }
  }

  node->type = ty;
  add_lvar(node->token, node->name, ty);

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
  Node* node = new_node(NEXPR_STMT, lt(0));
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
    Node* node = new_node(NRETURN, tokens->ptr[p - 1]);
    node->expr = expr();
    if (!match(TSEMICOLON)) {
      parse_error(";", lt(0));
    }
    return node;
  } else if (match_ident("if")) {
    Token* if_token = tokens->ptr[p - 1];

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
      Node* node = new_node(NIFELSE, if_token);
      node->cond = cond;
      node->then = then;
      node->els = els;
      return node;
    } else {
      Node* node = new_node(NIF, if_token);
      node->cond = cond;
      node->then = then;
      return node;
    }
  } else if (match_ident("while")) {
    Node* node = new_node(NWHILE, tokens->ptr[p - 1]);

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
    Node* node = new_node(NFOR, tokens->ptr[p - 1]);

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
    Node* node = new_node(NBLOCK, tokens->ptr[p - 1]);
    node->stmts = new_vec();

    local_env = new_env(local_env); // start scope

    while (la(0) != TRBRACE) {
      vec_push(node->stmts, statement());
    }

    if (!match(TRBRACE)) {
      parse_error("}", lt(0));
    }

    local_env = local_env->prev; // end scope

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

  char* name = lt(0)->ident;

  if (!match(TIDENT)) {
    parse_error("function name", lt(0));
  }

  if (!match(TLPAREN)) {
    if (match(TLBRACK)) {
      Type* array_ty = new_type();
      array_ty->ty = TY_PTR;
      array_ty->ptr_to = ret_type;
      array_ty->array_size = lt(0)->integer;
      ret_type = array_ty;
      if (!match(TINT)) {
        parse_error("integer", lt(0));
      }
      if (!match(TRBRACK)) {
        parse_error("]", lt(0));
      }
    }

    add_gvar(name, ret_type);
    if (match(TSEMICOLON)) {
      return funcdef();
    } else {
      parse_error("( or ;", lt(0));
    }
  }

  Vector* params = new_vec();
  local_env = new_env(local_env); // start scope
  local_size = 0;

  for (;;) {
    if (is_typename(lt(0))) {
      Type* ty = type_specifier();
      Node* param_decl = declarator(ty); // NDEFVAR
      Var* param = find_var(param_decl->token, param_decl->name);

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

  local_env = local_env->prev; // end scope

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
  strs = new_vec();
  local_env = NULL;
  global_env = new_map();

  Program* prog = calloc(1, sizeof(Program));
  prog->funcs = new_vec();

  while (!match(TEOF)) {
    vec_push(prog->funcs, funcdef());
  }

  prog->strs = strs;
  prog->globals = global_env;

  return prog;
}
