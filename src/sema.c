#include "hoc.h"

static Vector* funcs; // Vector(Function*)
static Vector* switches; // Vector(Node*(NSWITCH))
static int case_label_id;

static char* case_label(void) {
  char* label = format(".case%d", case_label_id);
  case_label_id++;
  return label;
}

static void type_error(Type* expected, Node* node) {
  bad_token(node->token, format("expected %s, but got %s\n", show_type(expected), show_type(type_of(node))));
}

// 各Nodeの.typeを埋める
void sema(Program* prog) {
  for (GVar* gvar = prog->globals; gvar != NULL; gvar = gvar->next) {
    walk(gvar->data);
  }

  funcs = prog->funcs;
  switches = new_vec();
  for (size_t i = 0; i < prog->funcs->length; i++) {
    Function* fn = prog->funcs->ptr[i];
    if (fn->body)
      walk(fn->body);
  }
}

static Type* ret_type(char* name) {
  for (size_t i = 0; i < funcs->length; i++) {
    Function* fn = funcs->ptr[i];
    if (streq(name, fn->name)) {
      return fn->ret_type;
    }
  }
  eprintf("undefined function %s\n", name);
  return int_type();
}

static int is_integer_type(Type* ty) {
  return (ty->ty == TY_CHAR) || (ty->ty == TY_INT) || (ty->ty == TY_LONG) || (ty->ty == TY_PTR);
}

static int is_assignable(Type* lhs, Type* rhs) {
  return (lhs->ty == rhs->ty) || (is_integer_type(lhs) && is_integer_type(rhs));
}

static int is_equal_type(Type* t1, Type* t2) {
  return (is_integer_type(t1) && is_integer_type(t2)) || t1->ty == t2->ty;
}

static void rewrite_arith_node(Node* node) {
  /* ポインタ演算を通常の整数演算に置き換える */
  if (type_of(node->lhs)->ty == TY_PTR && type_of(node->rhs)->ty != TY_PTR) {
    Node* rhs = node->rhs;
    Node* new = new_node(NMUL, rhs->token);
    new->lhs = rhs;
    new->rhs = new_node(NINT, rhs->token);
    new->rhs->integer = size_of(type_of(node->lhs)->ptr_to);
    new->rhs->type = ptr_to(type_of(node->lhs)->ptr_to);
    new->type = new->rhs->type;
    node->rhs = new;
  } else if (type_of(node->rhs)->ty == TY_PTR && type_of(node->lhs)->ty != TY_PTR) {
    Node* lhs = node->lhs;
    Node* new = new_node(NMUL, lhs->token);
    new->lhs = lhs;
    new->rhs = new_node(NINT, lhs->token);
    new->rhs->integer = size_of(type_of(node->rhs)->ptr_to);
    new->rhs->type = ptr_to(type_of(node->lhs)->ptr_to);
    new->type = new->rhs->type;
    node->lhs = new;
  }

  /* 左辺と右辺のサイズが大きい方の型をnodeに割り当てる */
  if (size_of(type_of(node->lhs)) >= size_of(type_of(node->rhs))) {
    node->type = type_of(node->lhs);
  } else {
    node->type = type_of(node->rhs);
  }

  if (node->type->array_size != 0) {
    /* 配列型になった場合はポインタ型に変換する */
    node->type = clone_type(node->type);
    node->type->array_size = 0;
  }
}

void walk(Node* node) {
  if (!node) {
    return;
  }

  switch (node->tag) {
  case NINT: {
    break;
  }
  case NVAR: {
    break;
  }
  case NGVAR: {
    break;
  }
  case NADD: {
    walk(node->lhs);
    walk(node->rhs);

    if (!(is_integer_type(type_of(node->lhs)))) {
      type_error(int_type(), node->lhs);
    }
    if (!(is_integer_type(type_of(node->rhs)))) {
      type_error(int_type(), node->rhs);
    }

    rewrite_arith_node(node);
    break;
  }
  case NSUB: {
    walk(node->lhs);
    walk(node->rhs);

    if (!(is_integer_type(node->lhs->type))) {
      type_error(int_type(), node->lhs);
    }
    if (!(is_integer_type(node->rhs->type))) {
      type_error(int_type(), node->rhs);
    }

    rewrite_arith_node(node);
    break;
  }
  case NMUL: {
    walk(node->lhs);
    walk(node->rhs);

    if (!(is_integer_type(node->lhs->type))) {
      type_error(int_type(), node->lhs);
    }
    if (!(is_integer_type(node->rhs->type))) {
      type_error(int_type(), node->rhs);
    }

    /* 左辺と右辺のサイズが大きい方の型をnodeに割り当てる */
    if (size_of(type_of(node->lhs)) >= size_of(type_of(node->rhs))) {
      node->type = type_of(node->lhs);
    } else {
      node->type = type_of(node->rhs);
    }

    if (node->type->array_size != 0) {
      /* 配列型になった場合はポインタ型に変換する */
      node->type = clone_type(node->type);
      node->type->array_size = 0;
    }

    break;
  }
  case NDIV: {
    walk(node->lhs);
    walk(node->rhs);

    if (!(is_integer_type(node->lhs->type))) {
      type_error(int_type(), node->lhs);
    }
    if (!(is_integer_type(node->rhs->type))) {
      type_error(int_type(), node->rhs);
    }

    node->type = node->lhs->type;
    break;
  }
  case NMOD: {
    walk(node->lhs);
    walk(node->rhs);

    if (!(is_integer_type(node->lhs->type))) {
      type_error(int_type(), node->lhs);
    }
    if (!(is_integer_type(node->rhs->type))) {
      type_error(int_type(), node->rhs);
    }

    node->type = node->lhs->type;
    break;
  }
  case NLT: {
    walk(node->lhs);
    walk(node->rhs);

    if (!(is_integer_type(node->lhs->type))) {
      type_error(int_type(), node->lhs);
    }
    if (!(is_integer_type(node->rhs->type))) {
      type_error(int_type(), node->rhs);
    }

    node->type = int_type();
    break;
  }
  case NLE: {
    walk(node->lhs);
    walk(node->rhs);

    if (!(is_integer_type(node->lhs->type))) {
      type_error(int_type(), node->lhs);
    }
    if (!(is_integer_type(node->rhs->type))) {
      type_error(int_type(), node->rhs);
    }

    node->type = int_type();
    break;
  }
  case NGT: {
    walk(node->lhs);
    walk(node->rhs);

    if (!(is_integer_type(node->lhs->type))) {
      type_error(int_type(), node->lhs);
    }
    if (!(is_integer_type(node->rhs->type))) {
      type_error(int_type(), node->rhs);
    }

    node->type = int_type();
    break;
  }
  case NGE: {
    walk(node->lhs);
    walk(node->rhs);

    if (!(is_integer_type(node->lhs->type))) {
      type_error(int_type(), node->lhs);
    }
    if (!(is_integer_type(node->rhs->type))) {
      type_error(int_type(), node->rhs);
    }

    node->type = int_type();
    break;
  }
  case NEQ: {
    walk(node->lhs);
    walk(node->rhs);

    if (!is_equal_type(node->lhs->type, node->rhs->type)) {
      type_error(type_of(node->lhs), node->rhs);
    }

    node->type = int_type();
    break;
  }
  case NNE: {
    walk(node->lhs);
    walk(node->rhs);

    if (!is_equal_type(node->lhs->type, node->rhs->type)) {
      type_error(type_of(node->lhs), node->rhs);
    }

    node->type = int_type();
    break;
  }
  case NNOT: {
    walk(node->expr);
    if (!is_integer_type(node->expr->type)) {
      type_error(int_type(), node->expr);
    }
    node->type = int_type();
    break;
  }
  case NLOGAND: {
    walk(node->lhs);
    walk(node->rhs);
    node->type = node->rhs->type;
    break;
  }
  case NLOGOR: {
    walk(node->lhs);
    walk(node->rhs);
    node->type = node->rhs->type;
    break;
  }
  case NCOMMA: {
    walk(node->lhs);
    walk(node->rhs);
    node->type = node->rhs->type;
    break;
  }
  case NDEFVAR: {
    node->type = NULL;
    break;
  }
  case NASSIGN: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = node->rhs->type;

    if (!is_assignable(type_of(node->lhs), type_of(node->rhs))) {
      type_error(type_of(node->lhs), node->rhs);
    }

    break;
  }
  case NCALL: {
    for (size_t i = 0; i < node->args->length; i++) {
      walk(node->args->ptr[i]);
    }
    node->type = ret_type(node->name);
    break;
  }
  case NADDR: {
    walk(node->expr);

    node->type = new_type();
    node->type->ty = TY_PTR;
    node->type->ptr_to = node->expr->type;

    break;
  }
  case NDEREF: {
    walk(node->expr);

    if (node->expr->type->ty != TY_PTR) {
      type_error(ptr_to(void_type()), node->expr);
    }

    node->type = node->expr->type->ptr_to;
    break;
  }
  case NMEMBER: {
    walk(node->expr);
    node->type = field_type(node->expr->type->fields, node->name);
    break;
  }
  case NEXPR_STMT: {
    walk(node->expr);
    node->type = NULL;
    break;
  }
  case NRETURN: {
    walk(node->expr);
    node->type = NULL;
    break;
  }
  case NIF: {
    walk(node->cond);
    walk(node->then);
    node->type = NULL;
    break;
  }
  case NIFELSE: {
    walk(node->cond);
    walk(node->then);
    walk(node->els);
    node->type = NULL;
    break;
  }
  case NWHILE: {
    walk(node->cond);
    walk(node->body);
    node->type = NULL;
    break;
  }
  case NFOR: {
    walk(node->init);
    walk(node->cond);
    walk(node->step);
    walk(node->body);
    node->type = NULL;
    break;
  }
  case NBLOCK: {
    for (size_t i = 0; i < node->stmts->length; i++) {
      walk(node->stmts->ptr[i]);
    }
    node->type = NULL;
    break;
  }
  case NSIZEOF: {
    walk(node->expr);
    node->type = int_type();
    break;
  }
  case NSTRING: {
    node->type = ptr_to(char_type());
    break;
  }
  case NSWITCH: {
    walk(node->expr);
    node->cases = new_vec();
    vec_push(switches, node);
    walk(node->body);
    node->type = NULL;
    break;
  }
  case NCASE: {
    walk(node->expr);
    walk(node->body);
    node->type = NULL;

    node->name = case_label();
    Node* sw = vec_last(switches);
    vec_push(sw->cases, node);
    break;
  }
  case NDEFAULT: {
    walk(node->body);
    node->type = NULL;

    node->name = case_label();
    Node* sw = vec_last(switches);
    vec_push(sw->cases, node);
  }
  case NBREAK: {
    node->type = NULL;
    break;
  }
  case NLIST: {
    walk(node->lhs);
    walk(node->rhs);
    node->type = ptr_to(type_of(node->lhs));
    break;
  }
  }
}
