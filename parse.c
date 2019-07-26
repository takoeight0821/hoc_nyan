#include "hoc.h"

typedef struct LVar {
  struct LVar *next;
  char* name;
  Type* type;
  int offset;
} LVar;

typedef struct Tag {
  struct Tag* next;
  Type* type;
} Tag;

typedef struct TypeDef {
  struct TypeDef* next;
  char* name;
  Type* type;
} TypeDef;

typedef struct Enum {
  struct Enum* next;
  char* name;
  int val;
} Enum;

static Token* tokens;
static LVar* local_env;
static size_t local_size = 0;
static Tag* tag_env;
static Enum* enum_env;
static GVar* global_env;
static TypeDef* typedefs;
static Vector* strs;

static Node* new_var(Token* tok, char* name, Type* type, size_t offset) {
  Node* var = new_node(NVAR, tok);
  var->name = name;
  var->type = type;
  var->offset = offset;
  return var;
}

static Node* new_gvar(Token* tok, char* name, Type* type) {
  Node* gvar = new_node(NGVAR, tok);
  gvar->name = name;
  gvar->type = type;
  return gvar;
}

static Node* find_var(Token* tok, char* name) {
  Node* var = NULL;
  for (LVar* env = local_env; var == NULL && env != NULL; env = env->next) {
    if (streq(name, env->name)) {
      var = new_var(tok, env->name, env->type, env->offset);
    }
  }

  if (var == NULL) {
    for (GVar* env = global_env; var == NULL && env != NULL; env = env->next) {
      if (streq(name, env->name)) {
        var = new_gvar(tok, env->name, env->type);
      }
    }
  }

  if (var == NULL) {
    for (Enum* env = enum_env; var == NULL && env != NULL; env = env->next) {
      if (streq(name, env->name)) {
        var = new_node(NINT, tok);
        var->integer = env->val;
      }
    }
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

static void add_gvar(Token* tok, char* name, Type* type) {
  GVar* gvar = calloc(sizeof(GVar), 1);
  gvar->name = name;
  gvar->type = type;
  gvar->next = global_env;
  global_env = gvar;
}

static void add_enum(char* name, int val) {
  Enum* e = calloc(1, sizeof(Enum));
  e->name = name;
  e->val = val;
  e->next = enum_env;
  enum_env = e;
}

// 文字列リテラルのインターン
static size_t intern(char* str) {
  size_t offset = strs->length;
  vec_push(strs, str);
  return offset;
}

void add_typedef(char* name, Type* type) {
  TypeDef* td = calloc(1, sizeof(TypeDef));
  td->name = name;
  td->type = type;
  td->next = typedefs;
  typedefs = td;
}

void init_typedef(void) {
  add_typedef("void", void_type());
  add_typedef("char", char_type());
  add_typedef("int", int_type());
  add_typedef("long", long_type());
}

static Type* find_type(char* name) {
  for (TypeDef* td = typedefs; td != NULL; td = td->next) {
    if (streq(name, td->name)) {
      return td->type;
    }
  }
  return NULL;
}

static Type* find_tag(char* tag_name) {
  Type* ty = NULL;
  for (Tag* tag = tag_env; ty == NULL && tag != NULL; tag = tag->next) {
    if (streq(tag_name, tag->type->tag)) {
      ty = tag->type;
    }
  }

  if (ty == NULL) {
    ty = new_type();
    ty->ty = TY_STRUCT;
    ty->tag = tag_name;
  }

  return ty;
}

static void consume() {
  tokens = tokens->next;
}

static Token* lt(size_t i) {
  Token* t = tokens;
  for (; i > 0; i--) {
    t = t->next;
  }
  return t;
}

static enum TokenTag la(size_t i) {
  assert(lt(i));
  return lt(i)->tag;
}

static void parse_error(const char* expected, Token* actual) {
  bad_token(actual, format("%s expected", expected));
}

static Token* match(enum TokenTag tag) {
  if (la(0) == tag) {
    Token* t = tokens;
    consume();
    return t;
  } else {
    return NULL;
  }
}

static Token* match_keyword(char* name) {
  if (la(0) == TIDENT && streq(lt(0)->ident, name)) {
    Token* t = lt(0);
    consume();
    return t;
  } else {
    return NULL;
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
  for (Field* f = t->fields; f != NULL; f = f->next) {
    f->offset = offset;
    offset += size_of(f->type);
  }
}

static Type* type_specifier() {
  Type* ty = calloc(1, sizeof(Type));

  Token* tok;
  if ((tok = match_keyword("struct"))) {
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
      if (ty->fields != NULL) {
        ty = clone_type(ty); // 同名の異なる型の定義なのでクローンする
      }

      ty->fields = NULL;
      while(!match(TRBRACE)) {
        Node* field = declarator(type_specifier());
        Field* new = calloc(sizeof(Field), 1);
        new->name = field->name;
        new->type = field->type;
        new->next = ty->fields;
        ty->fields = new;
        if (!match(TSEMICOLON)) {
          parse_error(";", lt(0));
        }
      }

      set_field_offset(ty);
      Tag* tag = calloc(sizeof(Tag), 1);
      tag->type = ty;
      tag->next = tag_env;
      tag_env = tag;
    }

    if (ty->fields == NULL) {
      bad_token(tok, format("struct %s is not defined\n", tag));
    }

  } else if (match_keyword("enum")) {
    ty = int_type();

    match(TIDENT); // タグは読み飛ばす

    if (match(TLBRACE)) {
      int val = 0;
      while (!match(TRBRACE)) {
        char* name = lt(0)->ident;
        if (!match(TIDENT)) {
          parse_error("ident", lt(0));
        }
        add_enum(name, val);
        val++;
        if (la(0) != TRBRACE && !match(TCOMMA)) {
          parse_error(",", lt(0));
        }
      }
    }

  } else if (la(0) == TIDENT && find_type(lt(0)->ident)) {
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
  Token* tok;
  if ((tok = match(TDOT))) {
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
  Token* tok;
  if ((tok = match_keyword("sizeof"))) {
    Node* node = new_node(NSIZEOF, tok);
    node->expr = unary();
    return node;
  } else if (match(TPLUS)) {
    return postfix();
  } else if ((tok = match(TMINUS))) {
    Node* node = new_node(NMINUS, tok);
    Node* zero = new_node(NINT, tok);
    zero->integer = 0;
    node->lhs = zero;
    node->rhs = postfix();
    return node;
  } else if ((tok = match(TAND))) {
    Node* node = new_node(NADDR, tok);
    node->expr = postfix();
    return node;
  } else if ((tok = match(TASTERISK))) {
    Node* node = new_node(NDEREF, tok);
    node->expr = postfix();
    return node;
  } else {
    Node* node = postfix();

    if ((tok = match(TLBRACK))) {
      Node* offset = expr();
      if (!match(TRBRACK)) {
        parse_error("]", lt(0));
      }
      Node* deref = new_node(NDEREF, tok);
      deref->expr = new_node(NPLUS, tok);
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
  Token* tok;
  for (;;) {
    if ((tok = match(TEQ))) {
      Node* node = new_node(NEQ, tok);
      node->lhs = lhs;
      node->rhs = relational();
      lhs = node;
    } else if ((tok = match(TNE))) {
      Node* node = new_node(NNE, tok);
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
  Token* tok;
  for (;;) {
    if ((tok = match(TLT))) {
      Node* node = new_node(NLT, tok);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if ((tok = match(TLE))) {
      Node* node = new_node(NLE, tok);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if ((tok = match(TGT))) {
      Node* node = new_node(NGT, tok);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if ((tok = match(TGE))) {
      Node* node = new_node(NGE, tok);
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
  Token* tok;
  for (;;) {
    if ((tok = match(TASTERISK))) {
      Node* node = new_node(NMUL, tok);
      node->lhs = lhs;
      node->rhs = unary();
      lhs = node;
    } else if ((tok = match(TSLASH))) {
      Node* node = new_node(NDIV, tok);
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
  Token* tok;
  for (;;) {
    if ((tok = match(TPLUS))) {
      Node* node = new_node(NPLUS, tok);
      node->lhs = lhs;
      node->rhs = mul();
      lhs = node;
    } else if ((tok = match(TMINUS))) {
      Node* node = new_node(NMINUS, tok);
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
    for (TypeDef* td = typedefs; td != NULL; td = td->next) {
      if (streq(t->ident, td->name)) {
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
  if (find_type(node->name)) {
    bad_token(lt(0), format("%s is a type name.", node->name));
  }

  consume();

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
  Token* tok;
  if (is_typename(lt(0))) {
    return declaration();
  } else if ((tok = match_keyword("return"))) {
    Node* node = new_node(NRETURN, tok);
    node->expr = expr();
    if (!match(TSEMICOLON)) {
      parse_error(";", lt(0));
    }
    return node;
  } else if ((tok = match_keyword("if"))) {
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
      Node* node = new_node(NIFELSE, tok);
      node->cond = cond;
      node->then = then;
      node->els = els;
      return node;
    } else {
      Node* node = new_node(NIF, tok);
      node->cond = cond;
      node->then = then;
      return node;
    }
  } else if ((tok = match_keyword("while"))) {
    Node* node = new_node(NWHILE, tok);

    if (!match(TLPAREN)) {
      parse_error("(", lt(0));
    }
    node->cond = expr();
    if (!match(TRPAREN)) {
      parse_error(")", lt(0));
    }

    node->body = statement();

    return node;
  } else if ((tok = match_keyword("for"))) {
    Node* node = new_node(NFOR, tok);

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
  } else if ((tok = match(TLBRACE))) {
    Node* node = new_node(NBLOCK, tok);
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
  add_typedef(lt(0)->ident, ty);
  consume();
  if (!match(TSEMICOLON)) {
    parse_error(";", lt(0));
  }
}

Program* parse(Token* t) {
  tokens = t;
  strs = new_vec();

  init_typedef();

  Program* prog = calloc(1, sizeof(Program));
  prog->funcs = new_vec();

  while (tokens) {
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
