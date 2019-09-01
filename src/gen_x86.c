#include "hoc.h"

static char* regs[NUM_REGS] = { "%r10", "%r11", "%rbx", "%r12", "%r13", "%r14", "%r15" };

#ifndef __hoc__
__attribute__((format(printf, 1, 2))) static void emit(char *fmt, ...);
#endif

static void emit(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  printf("\t");
  vprintf(fmt, ap);
  printf("\n");
}

static void emit_const(Type* type, Node* node) {
  if (!node) {
    return;
  }

  switch (node->tag) {
  case NINT: {
    if (size_of(type) == 1) {
      emit(".byte %d", node->integer);
    } else if (size_of(type) == 4) {
      emit(".int %d", node->integer);
    } else if (size_of(type) == 8) {
      emit(".quad %d", node->integer);
    } else {
      bad_token(node->token, "emit error: emit_const(invalid size)");
    }
    break;
  }
  case NGVAR: {
    if (size_of(type) == 1) {
      emit(".byte %s", node->name);
    } else if (size_of(type) == 4) {
      emit(".int %s", node->name);
    } else if (size_of(type) == 8) {
      emit(".quad %s", node->name);
    } else {
      bad_token(node->token, "emit error: emit_const(invalid size)");
    }
    break;
  }
  default:
    bad_token(node->token, "emit error: value is not constant");
  }
}

static void emit_function(IFunc* func) {
}

void gen_x86(IProgram* prog) {
  puts(".data");
  for (GVar* gvar = prog->globals; gvar != NULL; gvar = gvar->next) {
    if (gvar->init && !gvar->is_extern) {
      printf("%s:\n", gvar->name);
      emit_const(gvar->type, gvar->init);
    } else if (gvar->inits && !gvar->is_extern) {
      printf("%s:\n", gvar->name);
      for (size_t i = 0; i < gvar->inits->length; i++) {
        emit_const(gvar->type->ptr_to, gvar->inits->ptr[i]);
      }
    }
  }

  puts(".bss");
  for (GVar* gvar = prog->globals; gvar != NULL; gvar = gvar->next) {
    if (gvar->init == NULL && gvar->inits == NULL && !gvar->is_extern) {
      printf("%s:\n", gvar->name);
      emit(".zero %zu", size_of(gvar->type));
    }
  }

  puts(".text");
  for (size_t i = 0; i < prog->ifuncs->length; i++) {
    emit_function(prog->ifuncs->ptr[i]);
  }
}
