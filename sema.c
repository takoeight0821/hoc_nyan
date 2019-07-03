#include "hoc.h"

Vector* funcs; // Vector(Function*)

void walk(Node*);

// 各Nodeの.typeを埋める
void sema(Program* prog) {
  funcs = prog->funcs;
  for (size_t i = 0; i < prog->funcs->length; i++) {
    Function* fn = prog->funcs->ptr[i];
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
  return NULL;
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
    break;
  }
  case NMINUS: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = node->lhs->type;
    break;
  }
  case NMUL: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = node->lhs->type;
    break;
  }
  case NDIV: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = node->lhs->type;
    break;
  }
  case NLT: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;
    break;
  }
  case NLE: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;
    break;
  }
  case NGT: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;
    break;
  }
  case NGE: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;
    break;
  }
  case NEQ: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;
    break;
  }
  case NNE: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = new_type();
    node->type->ty = TY_INT;
    break;
  }
  case NDEFVAR: {
    break;
  }
  case NASSIGN: {
    walk(node->lhs);
    walk(node->rhs);

    node->type = node->rhs->type;
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
  }
}
