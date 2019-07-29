#include "hoc.h"

static Vector* funcs; // Vector(Function*)

static void walk(Node*);

static void type_error(Type* expected, Node* node) {
  bad_token(node->token, format("expected %s, but got %s\n", show_type(expected), show_type(type_of(node))));
}

// 各Nodeの.typeを埋める
void sema(Program* prog) {
  funcs = prog->funcs;
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

static void walk(Node* node) {
  switch (node->tag) {
  case NINT: {
    node->type = int_type();
    break;
  }
  case NVAR: {
    break;
  }
  case NGVAR: {
    break;
  }
  case NPLUS: {
    walk(node->lhs);
    walk(node->rhs);

    if (!(node->lhs->type->ty == TY_PTR || is_integer_type(node->lhs->type))) {
      type_error(int_type(), node->lhs);
    }
    if (!(is_integer_type(node->rhs->type))) {
      type_error(int_type(), node->rhs);
    }

    if (node->lhs->type->ty == TY_PTR && node->rhs->type->ty == TY_INT) {
      // ポインタの加算に対応
      // ptr + n -> ptr + sizeof(typeof(*ptr)) * n
      Node* new_rhs = new_node(NMUL, node->rhs->token);
      new_rhs->lhs = node->rhs;
      new_rhs->rhs = new_node(NINT, node->rhs->token);
      new_rhs->rhs->integer = size_of(node->lhs->type->ptr_to);
      walk(new_rhs);
      node->rhs = new_rhs;

      node->type = ptr_to(node->lhs->type->ptr_to);
    } else {
      node->type = int_type();
    }

    break;
  }
  case NMINUS: {
    walk(node->lhs);
    walk(node->rhs);

    if (!(node->lhs->type->ty == TY_PTR || is_integer_type(node->lhs->type))) {
      type_error(int_type(), node->lhs);
    }
    if (!(is_integer_type(node->rhs->type))) {
      type_error(int_type(), node->rhs);
    }

    if (node->lhs->type->ty == TY_PTR && node->rhs->type->ty == TY_INT) {
      // ポインタの減算に対応
      // ptr - n -> ptr - sizeof(typeof(*ptr)) * n
      Node* new_rhs = new_node(NMUL, node->rhs->token);
      new_rhs->lhs = node->rhs;
      new_rhs->rhs = new_node(NINT, node->rhs->token);
      new_rhs->rhs->integer = size_of(node->lhs->type->ptr_to);
      walk(new_rhs);
      node->rhs = new_rhs;

      node->type = ptr_to(node->lhs->type->ptr_to);
    } else {
      node->type = int_type();
    }

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

    node->type = node->lhs->type;
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
  case NDEFVAR: {
    if (node->lhs) {
      walk(node->lhs);
    }
    if (node->rhs) {
      walk(node->rhs);
    }
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
  }
}
