#include "hoc.h"

Vector* funcs; // Vector(Function*)

void walk(Node*);

// 各Nodeの.typeを埋める
void sema(Program* prog) {
  funcs = prog->funcs;
  for (size_t i = 0; i < prog->funcs->length; i++) {
    Function* fn = prog->funcs->ptr[i];
    if (fn->body)
      walk(fn->body);
  }
}

Type* ret_type(char* name) {
  for (size_t i = 0; i < funcs->length; i++) {
    Function* fn = funcs->ptr[i];
    if (streq(name, fn->name)) {
      return fn->ret_type;
    }
  }
  Type* t = new_type();
  t->ty = TY_INT;
  return t;
}

void walk(Node* node) {
  assert(node);

  switch (node->tag) {
  case NINT: {
    node->type = new_type();
    node->type->ty = TY_INT;
    break;
  }
  case NVAR: {
    node->type = node->var->type;
    break;
  }
  case NPLUS: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = node->lhs->type;

    assert((node->lhs->type->ty == TY_PTR && node->rhs->type->ty == TY_INT) ||
           (node->lhs->type->ty == TY_INT && node->rhs->type->ty == TY_INT));

    if (node->type->ty == TY_PTR && node->rhs->type->ty == TY_INT) {
      // ポインタの加算に対応
      // ptr + n -> ptr + sizeof(typeof(*ptr)) * n
      Node* new_rhs = new_node(NMUL);
      new_rhs->lhs = node->rhs;
      new_rhs->rhs = new_node(NINT);
      new_rhs->rhs->integer = size_of(node->type->ptr_to);
      walk(new_rhs);
      node->rhs = new_rhs;
    }
    break;
  }
  case NMINUS: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = node->lhs->type;

    assert((node->lhs->type->ty == TY_PTR && node->rhs->type->ty == TY_INT) ||
           (node->lhs->type->ty == TY_INT && node->rhs->type->ty == TY_INT));

    if (node->type->ty == TY_PTR && node->rhs->type->ty == TY_INT) {
      // ポインタの減算に対応
      // ptr - n -> ptr - sizeof(typeof(*ptr)) * n
      Node* new_rhs = new_node(NMUL);
      new_rhs->lhs = node->rhs;
      new_rhs->rhs = new_node(NINT);
      new_rhs->rhs->integer = size_of(node->type->ptr_to);
      walk(new_rhs);
      node->rhs = new_rhs;
    }
    break;
  }
  case NMUL: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = node->lhs->type;

    assert(node->lhs->type->ty == TY_INT && node->rhs->type->ty == TY_INT);
    break;
  }
  case NDIV: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = node->lhs->type;

    assert(node->lhs->type->ty == TY_INT && node->rhs->type->ty == TY_INT);
    break;
  }
  case NLT: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;

    assert(node->lhs->type->ty == TY_INT && node->rhs->type->ty == TY_INT);
    break;
  }
  case NLE: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;

    assert(node->lhs->type->ty == TY_INT && node->rhs->type->ty == TY_INT);
    break;
  }
  case NGT: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;

    assert(node->lhs->type->ty == TY_INT && node->rhs->type->ty == TY_INT);
    break;
  }
  case NGE: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;

    assert(node->lhs->type->ty == TY_INT && node->rhs->type->ty == TY_INT);
    break;
  }
  case NEQ: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;

    assert(node->lhs->type->ty == node->rhs->type->ty);
    break;
  }
  case NNE: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;

    assert(node->lhs->type->ty == node->rhs->type->ty);
    break;
  }
  case NDEFVAR: {
    break;
  }
  case NASSIGN: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = node->rhs->type;

    assert(node->lhs->type->ty == node->rhs->type->ty);
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
  }
}
