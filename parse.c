#include "hoc.h"

typedef struct LVar {
  struct LVar *next;
  char* name;
  Type* type;
  int offset;
} LVar;

typedef struct LTag {
  struct LTag* next;
  Type* ty;
} LTag;

static Vector* tokens;
static size_t p = 0; // 次の字句のインデックス
static LVar* local_env;
static size_t local_size = 0;
static LTag* tag_env;
static Map* global_env; // Map<Node*>
static Map* type_map; // Map<Type*>
static Vector* strs;

static Node* new_var(Token* tok, char* name, Type* type, size_t offset) {
  Node* var = new_node(NVAR, tok);
  var->name = name;
  var->type = type;
  var->offset = offset;
  return var;
}

static Node* find_var(Token* tok, char* name) {
  Node* var = NULL;
  for (LVar* env = local_env; var == NULL && env != NULL; env = env->next) {
    if (streq(name, env->name)) {
      var = new_var(tok, env->name, env->type, env->offset);
    }
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
  local_size += size_of(ty);
  LVar* lvar = calloc(sizeof(LVar), 1);
  lvar->name = name;
  lvar->type = ty;
  lvar->offset = local_size;
  lvar->next = local_env;
  local_env = lvar;
}

static Node* new_gvar(Token* tok, char* name, Type* type) {
  Node* gvar = new_node(NGVAR, tok);
  gvar->name = name;
  gvar->type = type;
  return gvar;
}

static void add_gvar(Token* tok, char* name, Type* type) {
  if (map_has_key(global_env, name)) {
    error("%s is already defined\n", name);
  }
  map_put(global_env, name, new_gvar(tok, name, type));
}

// 文字列リテラルのインターン
static size_t intern(char* str) {
  size_t offset = strs->length;
  vec_push(strs, str);
  return offset;
}

void init_type_map(void) {
  map_put(type_map, "void", void_type());
  map_put(type_map, "char", char_type());
  map_put(type_map, "int", int_type());
  map_put(type_map, "long", long_type());
}

static Type* find_type(char* name) {
  return map_get(type_map, name);
}

static Type* find_tag(char* tag) {
  Type* ty = NULL;
  for (LTag* ltag = tag_env; ty == NULL && ltag != NULL; ltag = ltag->next) {
    if (streq(tag, ltag->ty->tag)) {
      ty = ltag->ty;
    }
  }

  if (ty == NULL) {
    ty = new_type();
    ty->ty = TY_STRUCT;
    ty->tag = tag;
  }

  return ty;
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

static int match_keyword(char* name) {
  if (la(0) == TIDENT && streq(lt(0)->ident, name)) {
    consume();
    return 1;
  } else {
    return 0;
  }
}

static Node* variable();
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
static Node* declarator(Type* ty);

void set_field_offset(Type* t) {
  size_t offset = 0;
  for (size_t i = 0; i < t->struct_fields->keys->length; i++) {
    // field_offsetを書き換えるのでクローン
    Type* field = clone_type(t->struct_fields->vals->ptr[i]);
    field->field_offset = offset;
    t->struct_fields->vals->ptr[i] = field;
    offset += size_of(field);
  }
}

static Type* type_specifier() {
  Type* ty = calloc(1, sizeof(Type));

  if (match_keyword("struct")) {
    Token* tok = tokens->ptr[p - 1];
    char* tag;

    if (la(0) == TIDENT) {
      tag = lt(0)->ident;
      ty = find_tag(tag);
      consume();
    } else {
      parse_error("ident", lt(0));
    }

    // struct definition
    if (match(TLBRACE)) {
      if (ty->struct_fields != NULL) {
        ty = clone_type(ty); // 同名の異なる型の定義なのでクローンする
      }

      ty->struct_fields = new_map();
      while(!match(TRBRACE)) {
        Node* field = declarator(type_specifier());
        map_put(ty->struct_fields, field->name, field->type);
        if (!match(TSEMICOLON)) {
          parse_error(";", lt(0));
        }
      }

      set_field_offset(ty);
      LTag* ltag = calloc(sizeof(LTag), 1);
      ltag->ty = ty;
      ltag->next = tag_env;
      tag_env = ltag;
    }

    if (ty->struct_fields == NULL) {
      bad_token(tok, format("struct %s is not defined\n", tag));
    }

  } else if (la(0) == TIDENT && map_has_key(type_map, lt(0)->ident)) {
    ty = find_type(lt(0)->ident);
    consume();
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
    Node* node = variable();
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

static Node* postfix() {
  Node* e = term();
  if (match(TDOT)) {
    Token* tok = tokens->ptr[p - 1];
    char* name = lt(0)->ident;
    if (!match(TIDENT)) {
      parse_error("ident", lt(0));
    }
    Node* node = new_node(NMEMBER, tok);
    node->expr = e;
    node->name = name;
    return node;
  } else {
    return e;
  }
}

static Node* unary() {
  if (match_keyword("sizeof")) {
    Node* node = new_node(NSIZEOF, tokens->ptr[p - 1]);
    node->expr = unary();
    return node;
  } else if (match(TPLUS)) {
    return postfix();
  } else if (match(TMINUS)) {
    Node* node = new_node(NMINUS, tokens->ptr[p - 1]);
    Node* zero = new_node(NINT, tokens->ptr[p - 1]);
    zero->integer = 0;
    node->lhs = zero;
    node->rhs = postfix();
    return node;
  } else if (match(TAND)) {
    Node* node = new_node(NADDR, tokens->ptr[p - 1]);
    node->expr = postfix();
    return node;
  } else if (match(TASTERISK)) {
    Node* node = new_node(NDEREF, tokens->ptr[p - 1]);
    node->expr = postfix();
    return node;
  } else {
    Node* node = postfix();

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

static Node* variable() {
  assert(la(0) == TIDENT);
  Node* node = find_var(lt(0), lt(0)->ident);
  consume();
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
  if (t->tag == TIDENT && streq(t->ident, "struct")) {
    return 1;
  }

  if (t->tag == TIDENT) {
    for (size_t i = 0; i < type_map->keys->length; i++) {
      if (streq(t->ident, type_map->keys->ptr[i])) {
        return 1;
      }
    }
  }
  return 0;
}

static Node* direct_decl(Type* ty) {
  if (la(0) != TIDENT) {
    parse_error("ident", lt(0));
  }
  Node* node = new_node(NDEFVAR, lt(0));
  node->name = lt(0)->ident;
  consume();

  if (map_has_key(type_map, node->name)) {
    bad_token(tokens->ptr[p - 1], format("%s is a type name.", node->name));
  }

  node->type = ty;

  if (match(TLBRACK)) {
    Type* array_ty = new_type();
    array_ty->ty = TY_PTR;
    array_ty->ptr_to = ty;
    array_ty->array_size = lt(0)->integer;
    node->type = array_ty;
    if (!match(TINT)) {
      parse_error("integer", lt(0));
    }
    if (!match(TRBRACK)) {
      parse_error("]", lt(0));
    }
  }

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
  Node* decl = declarator(type_specifier());

  if (!match(TSEMICOLON)) {
    parse_error(";", lt(0));
  }

  add_lvar(decl->token, decl->name, decl->type);

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
  } else if (match_keyword("return")) {
    Node* node = new_node(NRETURN, tokens->ptr[p - 1]);
    node->expr = expr();
    if (!match(TSEMICOLON)) {
      parse_error(";", lt(0));
    }
    return node;
  } else if (match_keyword("if")) {
    Token* if_token = tokens->ptr[p - 1];

    if (!match(TLPAREN)) {
      parse_error("(", lt(0));
    }
    Node* cond = expr();
    if (!match(TRPAREN)) {
      parse_error(")", lt(0));
    }

    Node* then = statement();

    if (match_keyword("else")) {
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
  } else if (match_keyword("while")) {
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
  } else if (match_keyword("for")) {
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

    LVar* tmp = local_env; // start scope

    while (la(0) != TRBRACE) {
      vec_push(node->stmts, statement());
    }

    if (!match(TRBRACE)) {
      parse_error("}", lt(0));
    }

    local_env = tmp; // end scope

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
  Token* tok = lt(0);

  if (!match(TIDENT)) {
    if (match(TSEMICOLON)) {
      // type definition
      return funcdef();
    }
    parse_error("function name or ;", lt(0));
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

    add_gvar(tok, name, ret_type);
    if (match(TSEMICOLON)) {
      return funcdef();
    } else {
      parse_error("( or ;", lt(0));
    }
  }

  Vector* params = new_vec();
  LVar* tmp = local_env; // start scope
  local_size = 0;

  for (;;) {
    if (is_typename(lt(0))) {
      Node* param_decl = declarator(type_specifier()); // NDEFVAR
      add_lvar(param_decl->token, param_decl->name, param_decl->type);
      Node* param = find_var(param_decl->token, param_decl->name);

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

  local_env = tmp; // end scope

  Function* func = calloc(1, sizeof(Function));
  func->name = name;
  func->ret_type = ret_type;
  func->body = body;
  func->params = params;
  func->local_size = local_size;

  return func;
}

void type_alias_def(void) {
  Type* ty = type_specifier();
  if (la(0) != TIDENT) {
    parse_error("ident", lt(0));
  }
  map_put(type_map, lt(0)->ident, ty);
  consume();
  if (!match(TSEMICOLON)) {
    parse_error(";", lt(0));
  }
}

Program* parse(Vector* token_vec) {
  tokens = token_vec;
  strs = new_vec();
  local_env = NULL;
  tag_env = NULL;
  global_env = new_map();
  type_map = new_map();

  init_type_map();

  Program* prog = calloc(1, sizeof(Program));
  prog->funcs = new_vec();

  while (!match(TEOF)) {
    if (match_keyword("typedef")) {
      type_alias_def();
    } else {
      vec_push(prog->funcs, funcdef());
    }
  }

  prog->strs = strs;
  prog->globals = global_env;

  return prog;
}
