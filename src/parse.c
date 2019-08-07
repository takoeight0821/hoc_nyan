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
  Node* val;
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
  for (LVar* env = local_env; env != NULL; env = env->next) {
    if (streq(name, env->name)) {
      return new_var(tok, env->name, env->type, env->offset);
    }
  }

  for (GVar* env = global_env; env != NULL; env = env->next) {
    if (streq(name, env->name)) {
      return new_gvar(tok, env->name, env->type);
    }
  }

  for (Enum* env = enum_env; env != NULL; env = env->next) {
    if (streq(name, env->name)) {
      Node* val = clone_node(env->val);
      val->token = tok;
      return val;
    }
  }

  bad_token(tok, format("%s is not defined\n", name));
  return NULL;
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
  e->val = new_node(NINT, NULL);
  e->val->integer = val;
  e->next = enum_env;
  enum_env = e;
}

// 文字列リテラルのインターン
static size_t intern(char* str) {
  size_t id = strs->length;
  vec_push(strs, str);
  return id;
}

static void add_typedef(char* name, Type* type) {
  TypeDef* td = calloc(1, sizeof(TypeDef));
  td->name = name;
  td->type = type;
  td->next = typedefs;
  typedefs = td;
}

static void init_typedef(void) {
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
  for (Tag* tag = tag_env; tag != NULL; tag = tag->next) {
    if (streq(tag_name, tag->type->tag)) {
      return tag->type;
    }
  }

  Type* ty = new_type();
  ty->ty = TY_STRUCT;
  ty->tag = tag_name;
  return ty;
}

static void consume() {
  tokens = tokens->next;
}

static Token* lt(size_t i) {
  Token* t = tokens;
  // i回nextをたどるとi+1個目の要素になる
  for (; i > 0; i--) {
    t = t->next;
  }
  return t;
}

static enum TokenTag la(size_t i) {
  return lt(i)->tag;
}

static void parse_error(const char* expected, Token* actual) {
  bad_token(actual, format("%s expected", expected));
}

static Token* expect(enum TokenTag tag, char* expected) {
  if (la(0) == tag) {
    Token* t = tokens;
    consume();
    return t;
  }
  parse_error(expected, lt(0));
  return NULL;
}

static Token* match(char* name) {
  if (la(0) == TRESERVED && streq(lt(0)->ident, name)) {
    Token* t = lt(0);
    consume();
    return t;
  } else {
    return NULL;
  }
}

static bool eq_reserved(Token* token, char* name) {
  return (token->tag == TRESERVED && streq(token->ident, name));
}

static int is_typename(Token* t);
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
static Node* logical_and();
static Node* logical_or();
static Node* new_assign_node(Token* token, enum NodeTag op, Node* lhs, Node* rhs);
static Node* assign();
static Node* statement();
static Node* declarator(Type* ty);

static void set_field_offset(Type* t) {
  size_t offset = size_of(t);
  for (Field* f = t->fields; f != NULL; f = f->next) {
    offset -= size_of(f->type);
    f->offset = offset;
  }
}

static Type* type_specifier() {
  Type* ty = calloc(1, sizeof(Type));

  Token* tok;
  if ((tok = match("struct"))) {
    char* tag;

    if (la(0) == TIDENT) {
      tag = lt(0)->ident;
      ty = find_tag(tag);
      consume();
    } else {
      parse_error("ident", lt(0));
    }

    // struct definition
    if (match("{")) {
      if (ty->fields != NULL) {
        ty = clone_type(ty); // 同名の異なる型の定義なのでクローンする
      }

      ty->fields = NULL;
      while(!match("}")) {
        Node* field = declarator(type_specifier());
        Field* new = calloc(sizeof(Field), 1);
        new->name = field->name;
        new->type = field->type;
        new->next = ty->fields;
        ty->fields = new;
        if (!match(";")) {
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

  } else if (match("enum")) {
    ty = int_type();

    expect(TIDENT, "enum tag"); // タグは読み飛ばす

    if (match("{")) {
      int val = 0;
      while (true) {
        char* name = expect(TIDENT, "enum value")->ident;
        add_enum(name, val);
        val++;

        // 要素末尾の","を許す
        if (match("}")) {
          break;
        }
        if (!match(",")) {
          parse_error(",", lt(0));
        }
        if (match("}")) {
          break;
        }
      }
    }

  } else if ((la(0) == TIDENT || la(0) == TRESERVED) && find_type(lt(0)->ident)) {
    ty = find_type(lt(0)->ident);
    consume();
  } else {
    ty = NULL;
  }

  return ty;
}

static Node* term() {
  if (match("(")) {
    Node* node = expr();
    if (!match(")")) {
      parse_error(")", lt(0));
    }
    return node;
  } else if (la(0) == TIDENT && !eq_reserved(lt(1), "(")) {
    Node* node = variable();
    return node;
  } else if (la(0) == TIDENT) {
    Node* node = new_node(NCALL, lt(0));
    node->name = lt(0)->ident;
    node->args = new_vec();
    consume();
    if (!match("(")) {
      parse_error("(", lt(0));
    }
    if (match(")")) {
      return node;
    }

    for (;;) {
      vec_push(node->args, assign());
      if (match(")")) {
        return node;
      } else if (!match(",")) {
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
  Node* node = term();
  Token* tok;

  for (;;) {
    if ((tok = match("."))) {
      char* name = expect(TIDENT, "field name")->ident;
      Node* e = new_node(NMEMBER, tok);
      e->expr = node;
      e->name = name;
      node = e;
    } else if ((tok = match("->"))) {
      char* name = expect(TIDENT, "field name")->ident;
      Node* deref = new_node(NDEREF, tok);
      deref->expr = node;
      Node* e = new_node(NMEMBER, tok);
      e->expr = deref;
      e->name = name;
      node = e;
    } else if ((tok = match("["))) {
      Node* offset = expr();
      if (!match("]")) {
        parse_error("]", lt(0));
      }
      Node* e = new_node(NDEREF, tok);
      e->expr = new_node(NADD, tok);
      e->expr->lhs = node;
      e->expr->rhs = offset;
      node = e;
    } else if ((tok = match("++"))) {
      /*
       * a++ -> (a = a + 1, a - 1)
       */
      Node* one = new_node(NINT, tok);
      one->integer = 1;
      Node* assign = new_assign_node(tok, NADD, node, one);
      Node* value = new_node(NSUB, tok);
      value->lhs = node;
      value->rhs = one;
      node = new_node(NCOMMA, tok);
      node->lhs = assign;
      node->rhs = value;
    } else if ((tok = match("--"))) {
      /*
       * a-- -> (a = a - 1, a + 1)
       */
      Node* one = new_node(NINT, tok);
      one->integer = 1;
      Node* assign = new_assign_node(tok, NSUB, node, one);
      Node* value = new_node(NADD, tok);
      value->lhs = node;
      value->rhs = one;
      node = new_node(NCOMMA, tok);
      node->lhs = assign;
      node->rhs = value;
    } else {
      return node;
    }
  }
}

static Node* unary() {
  Token* tok;
  if ((tok = match("sizeof"))) {
    if (!match("(")) {
      parse_error("(", lt(0));
    }
    Node* node;
    if (is_typename(lt(0))) {
      // sizeof(type)は計算してしまってNINTノードにする
      Type* type = type_specifier();
      while (match("*")) {
        type = ptr_to(type);
      }
      node = new_node(NINT, tok);
      node->integer = size_of(type);
    } else {
      // sizeof(expr)はsemaで型付けしてemitで計算する
      node = new_node(NSIZEOF, tok);
      node->expr = unary();
    }
    if (!match(")")) {
      parse_error(")", lt(0));
    }
    return node;
  } else if (match("+")) {
    return postfix();
  } else if ((tok = match("-"))) {
    Node* node = new_node(NSUB, tok);
    Node* zero = new_node(NINT, tok);
    zero->integer = 0;
    node->lhs = zero;
    node->rhs = postfix();
    return node;
  } else if ((tok = match("&"))) {
    Node* node = new_node(NADDR, tok);
    node->expr = postfix();
    return node;
  } else if ((tok = match("*"))) {
    Node* node = new_node(NDEREF, tok);
    node->expr = postfix();
    return node;
  } else if ((tok = match("!"))) {
    Node* node = new_node(NNOT, tok);
    node->expr = postfix();
    return node;
  } else {
    Node* node = postfix();
    return node;
  }
}

static Node* variable() {
  Node* node = find_var(lt(0), expect(TIDENT, "variable")->ident);
  return node;
}

static Node* integer() {
  Node* node = new_node(NINT, lt(0));
  node->integer = expect(TINT, "integer")->integer;
  return node;
}

static Node* string() {
  Node* node = new_node(NSTRING, lt(0));
  char* str = expect(TSTRING, "string")->str;

  node->str_id = intern(str);
  return node;
}

static Node* logical_or() {
  Node* lhs = logical_and();
  Token* tok;
  for (;;) {
    if ((tok = match("||"))) {
      Node* node = new_node(NLOGOR, tok);
      node->lhs = lhs;
      node->rhs = logical_and();
      lhs = node;
    } else {
      return lhs;
    }
  }
}

static Node* logical_and() {
  Node* lhs = equality();
  Token* tok;
  for (;;) {
    if ((tok = match("&&"))) {
      Node* node = new_node(NLOGAND, tok);
      node->lhs = lhs;
      node->rhs = equality();
      lhs = node;
    } else {
      return lhs;
    }
  }
}

static Node* equality() {
  Node* lhs = relational();
  Token* tok;
  for (;;) {
    if ((tok = match("=="))) {
      Node* node = new_node(NEQ, tok);
      node->lhs = lhs;
      node->rhs = relational();
      lhs = node;
    } else if ((tok = match("!="))) {
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
    if ((tok = match("<"))) {
      Node* node = new_node(NLT, tok);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if ((tok = match("<="))) {
      Node* node = new_node(NLE, tok);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if ((tok = match(">"))) {
      Node* node = new_node(NGT, tok);
      node->lhs = lhs;
      node->rhs = add();
      lhs = node;
    } else if ((tok = match(">="))) {
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
    if ((tok = match("*"))) {
      Node* node = new_node(NMUL, tok);
      node->lhs = lhs;
      node->rhs = unary();
      lhs = node;
    } else if ((tok = match("/"))) {
      Node* node = new_node(NDIV, tok);
      node->lhs = lhs;
      node->rhs = unary();
      lhs = node;
    } else if ((tok = match("%"))) {
      Node* node = new_node(NMOD, tok);
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
    if ((tok = match("+"))) {
      Node* node = new_node(NADD, tok);
      node->lhs = lhs;
      node->rhs = mul();
      lhs = node;
    } else if ((tok = match("-"))) {
      Node* node = new_node(NSUB, tok);
      node->lhs = lhs;
      node->rhs = mul();
      lhs = node;
    } else {
      return lhs;
    }
  }
}

/*
 * a op= b -> a = a op b
 */
static Node* new_assign_node(Token* token, enum NodeTag op, Node* lhs, Node* rhs) {
  Node* node = new_node(NASSIGN, token);
  node->lhs = lhs;
  node->rhs = new_node(op, token);
  node->rhs->lhs = lhs;
  node->rhs->rhs = rhs;
  return node;
}

static Node* assign() {
  Node* node = logical_or();
  Token* token;

  if (match("=")) {
    Node* lhs = node;
    Node* rhs = assign();
    node = new_node(NASSIGN, lhs->token);
    node->lhs = lhs;
    node->rhs = rhs;
  } else if ((token = match("+="))) {
    node = new_assign_node(token, NADD, node, assign());
  } else if ((token = match("-="))) {
    node = new_assign_node(token, NSUB, node, assign());
  } else if ((token = match("*="))) {
    node = new_assign_node(token, NMUL, node, assign());
  }

  return node;
}

static Node* comma() {
  Node* lhs = assign();

  Token* tok;
  for (;;) {
    if ((tok = match(","))) {
      Node* node = new_node(NCOMMA, tok);
      node->lhs = lhs;
      node->rhs = assign();
      lhs = node;
    } else {
      return lhs;
    }
  }
}

static Node* expr() {
  return comma();
}

static int is_typename(Token* t) {
  if (t->tag == TRESERVED && streq(t->ident, "struct")) {
    return 1;
  }

  if (t->tag == TIDENT || t->tag == TRESERVED) {
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

  if (match("[")) {
    Type* array_ty = new_type();
    array_ty->ty = TY_PTR;
    array_ty->ptr_to = ty;
    array_ty->array_size = expect(TINT, "integer")->integer;
    node->type = array_ty;
    if (!match("]")) {
      parse_error("]", lt(0));
    }
  }

  return node;
}

static Node* declarator(Type* ty) {
  while (match("*")) {
    ty = ptr_to(ty);
  }
  return direct_decl(ty);
}

static Node* declaration() {
  // variable definition
  Node* decl = declarator(type_specifier());
  add_lvar(decl->token, decl->name, decl->type);

  Token* tok;
  if ((tok = match("="))) {
    decl->lhs = find_var(tok, decl->name);
    decl->rhs = assign();
  }

  return decl;
};

static Node* expr_stmt() {
  Node* node = new_node(NEXPR_STMT, lt(0));
  node->expr = expr();
  if (!match(";")) {
    parse_error(";", lt(0));
  }
  return node;
};

static Node* statement() {
  Token* tok;
  if (is_typename(lt(0))) {
    Node* node = declaration();
    if (!match(";")) {
      parse_error(";", lt(0));
    }
    return node;
  } else if ((tok = match("return"))) {
    Node* node = new_node(NRETURN, tok);
    node->expr = expr();
    if (!match(";")) {
      parse_error(";", lt(0));
    }
    return node;
  } else if ((tok = match("break"))) {
    Node* node = new_node(NBREAK, tok);
    if (!match(";")) {
      parse_error(";", lt(0));
    }
    return node;
  } else if ((tok = match("if"))) {
    if (!match("(")) {
      parse_error("(", lt(0));
    }
    Node* cond = expr();
    if (!match(")")) {
      parse_error(")", lt(0));
    }

    Node* then = statement();

    if (match("else")) {
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
  } else if ((tok = match("while"))) {
    Node* node = new_node(NWHILE, tok);

    if (!match("(")) {
      parse_error("(", lt(0));
    }
    node->cond = expr();
    if (!match(")")) {
      parse_error(")", lt(0));
    }

    node->body = statement();

    return node;
  } else if ((tok = match("for"))) {
    Node* node = new_node(NFOR, tok);

    LVar* tmp = local_env;
    if (!match("(")) {
      parse_error("(", lt(0));
    }

    if (is_typename(lt(0))) {
      node->init = declaration();
    } else {
      node->init = expr();
    }

    if (!match(";")) {
      parse_error(";", lt(0));
    }
    node->cond = expr();

    if (!match(";")) {
      parse_error(";", lt(0));
    }
    node->step = expr();

    if(!match(")")) {
      parse_error(")", lt(0));
    }
    node->body = statement();

    local_env = tmp;

    return node;
  } else if ((tok = match("switch"))) {
    Node* node = new_node(NSWITCH, tok);

    if (!match("(")) {
      parse_error("(", tok);
    }
    node->expr = expr();
    if (!match(")")) {
      parse_error(")", tok);
    }

    node->body = statement();

    return node;
  } else if ((tok = match("case"))) {
    Node* node = new_node(NCASE, tok);
    node->expr = logical_or(); // TODO: support conditional expression
    if (!match(":")) {
      parse_error(":", tok);
    }
    node->body = statement();
    return node;
  } else if ((tok = match("default"))) {
    Node* node = new_node(NDEFAULT, tok);
    if (!match(":")) {
      parse_error(":", tok);
    }
    node->body = statement();
    return node;
  } else if ((tok = match("{"))) {
    Node* node = new_node(NBLOCK, tok);
    node->stmts = new_vec();

    LVar* tmp = local_env; // start scope

    while (!eq_reserved(lt(0), "}")) {
      vec_push(node->stmts, statement());
    }

    if (!match("}")) {
      parse_error("}", lt(0));
    }

    local_env = tmp; // end scope

    return node;
  } else {
    return expr_stmt();
  }
}

static void global_var(void);

static Function* funcdef(bool is_static) {
  Token* back = lt(0);
  Type* ret_type = type_specifier();

  while (match("*")) {
    ret_type = ptr_to(ret_type);
  }

  char* name = lt(0)->ident;

  if (la(0) != TIDENT) {
    if (match(";")) {
      // type definition
      return NULL;
    }
    parse_error("function name or ;", lt(0));
  } else {
    consume(); // TIDENT
  }

  if (!match("(")) {
    tokens = back;
    global_var();
    return NULL;
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
      if (match(","))
        continue;
      if (match(")"))
        break;
      parse_error(", or )", lt(0));

    } else if (match(")")) {
      break;
    } else {
      parse_error(")", lt(0));
    }
  }

  if (match(";")) {
    Function* func = calloc(1, sizeof(Function));
    func->name = name;
    func->ret_type = ret_type;
    func->body = NULL;
    func->params = params;
    func->local_size = local_size;
    func->is_static = is_static;
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
  func->is_static = is_static;

  return func;
}

static void type_alias_def(void) {
  Type* ty = type_specifier();
  if (la(0) != TIDENT) {
    parse_error("ident", lt(0));
  }
  add_typedef(lt(0)->ident, ty);
  consume();
  if (!match(";")) {
    parse_error(";", lt(0));
  }
}

static void global_var(void) {
  Type* type = type_specifier();

  while (match("*")) {
    type = ptr_to(type);
  }

  Token* tok = lt(0);
  char* name = expect(TIDENT, "function name or ;")->ident;

  if (match("[")) {
    Type* array_ty = new_type();
    array_ty->ty = TY_PTR;
    array_ty->ptr_to = type;
    array_ty->array_size = expect(TINT, "integer")->integer;
    type = array_ty;
    if (!match("]")) {
      parse_error("]", lt(0));
    }
  }

  add_gvar(tok, name, type);
  if (!match(";")) {
    parse_error("global_var: ;", lt(0));
  }
}

static Function* toplevel() {
  if (match("typedef")) {
    type_alias_def();
    return NULL;
  } else if (match("extern")) {
    global_var();
    global_env->is_extern = true;
    return NULL;
  } else if (match("static")) {
    return funcdef(true);
  } else {
    return funcdef(false);
  }
}

Program* parse(Token* t) {
  tokens = t;
  strs = new_vec();

  init_typedef();

  Program* prog = calloc(1, sizeof(Program));
  prog->funcs = new_vec();

  while (tokens) {
    Function* func = toplevel();
    if (func) {
      vec_push(prog->funcs, func);
    }
  }

  prog->strs = strs;
  prog->globals = global_env;

  return prog;
}
