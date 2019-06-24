#include "hoc.h"

Node* walk(Vector* params, Node* node) {
  return node;
}

void sema(Program* prog) {
  for (size_t i = 0; i < prog->funcs->length; i++) {
    Function* fn = prog->funcs->ptr[i];
    fn->body = walk(fn->params, fn->body);
  }
}
