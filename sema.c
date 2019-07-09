#include "hoc.h"

Vector* funcs; // Vector(Function*)

void walk(Node*);

noreturn void type_error(Type* expected, Node* node) {
  bad_token(node->token, format("type error: expected %s, but got %s", show_type(expected), show_type(type_of(node))));
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

Type* int_type(void) {
  Type* t = new_type();
  t->ty = TY_INT;
  return t;
}

Type* char_type(void) {
  Type* t = new_type();
  t->ty = TY_CHAR;
  return t;
}

Type* ptr_ty(Type* ptr_to) {
  Type* t = new_type();
  t->ty = TY_PTR;
  t->ptr_to = ptr_to;
  return t;
}

Type* ret_type(char* name) {
  for (size_t i = 0; i < funcs->length; i++) {
    Function* fn = funcs->ptr[i];
    if (streq(name, fn->name)) {
      return fn->ret_type;
    }
  }
  return int_type();
}

int integer_type(Type* ty) {
  return (ty->ty == TY_CHAR) || (ty->ty == TY_INT);
}

int assignable(Type* lhs, Type* rhs) {
  return (lhs->ty == rhs->ty) || (integer_type(lhs) && integer_type(rhs));
}

void walk(Node* node) {
  assert(node);

  switch (node->tag) {
  case NINT: {
    node->type = int_type();
    break;
  }
  case NVAR: {
    node->type = node->var->type;
    break;
  }
  case NPLUS: {
    walk(node->lhs);
    walk(node->rhs);

    if (node->lhs->type->ty == TY_PTR || node->lhs->type->ty == TY_INT) {
      if (node->rhs->type->ty != TY_INT) {
        type_error(int_type(), node->rhs);
      }
    } else {
      type_error(int_type(), node->lhs);
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

      node->type = ptr_ty(node->lhs->type->ptr_to);
    } else {
      node->type = int_type();
    }

    break;
  }
  case NMINUS: {
    walk(node->lhs);
    walk(node->rhs);

    if (node->lhs->type->ty == TY_PTR || node->lhs->type->ty == TY_INT) {
      if (node->rhs->type->ty != TY_INT) {
        type_error(int_type(), node->rhs);
      }
    } else {
      type_error(int_type(), node->lhs);
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

      node->type = ptr_ty(node->lhs->type->ptr_to);
    } else {
      node->type = int_type();
    }

    break;
  }
  case NMUL: {
    walk(node->lhs);
    walk(node->rhs);

    if (node->lhs->type->ty != TY_INT) {
      type_error(int_type(), node->lhs);
    }
    if (node->rhs->type->ty != TY_INT) {
      type_error(int_type(), node->rhs);
    }

    node->type = node->lhs->type;

    break;
  }
  case NDIV: {
    walk(node->lhs);
    walk(node->rhs);

    if (node->lhs->type->ty != TY_INT) {
      type_error(int_type(), node->lhs);
    }
    if (node->rhs->type->ty != TY_INT) {
      type_error(int_type(), node->rhs);
    }

    node->type = node->lhs->type;

    break;
  }
  case NLT: {
    walk(node->lhs);
    walk(node->rhs);

    if (node->lhs->type->ty != TY_INT) {
      type_error(int_type(), node->lhs);
    }
    if (node->rhs->type->ty != TY_INT) {
      type_error(int_type(), node->rhs);
    }

    node->type = int_type();

    break;
  }
  case NLE: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;

    if (node->lhs->type->ty != TY_INT) {
      type_error(int_type(), node->lhs);
    }
    if (node->rhs->type->ty != TY_INT) {
      type_error(int_type(), node->rhs);
    }

    break;
  }
  case NGT: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;

    if (node->lhs->type->ty != TY_INT) {
      type_error(int_type(), node->lhs);
    }
    if (node->rhs->type->ty != TY_INT) {
      type_error(int_type(), node->rhs);
    }

    break;
  }
  case NGE: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;

    if (node->lhs->type->ty != TY_INT) {
      type_error(int_type(), node->lhs);
    }
    if (node->rhs->type->ty != TY_INT) {
      type_error(int_type(), node->rhs);
    }

    break;
  }
  case NEQ: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;

    if (node->lhs->type->ty != node->rhs->type->ty) {
      type_error(type_of(node->lhs), node->rhs);
    }

    break;
  }
  case NNE: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;

    if (node->lhs->type->ty != node->rhs->type->ty) {
      type_error(type_of(node->lhs), node->rhs);
    }

    break;
  }
  case NDEFVAR: {
    break;
  }
  case NASSIGN: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = node->rhs->type;

    if (!assignable(type_of(node->lhs), type_of(node->rhs))) {
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

    assert(node->type);
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

    node->type = new_type();
    node->type->ty = TY_INT;

    break;
  }
  case NSTRING: {
    node->type = ptr_ty(char_type());
    break;
  }
  }
}
