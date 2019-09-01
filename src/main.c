#include "hoc.h"

int main(int argc, char** argv)
{
  bool dump = false;
  bool use_ir = false;

  int opt;
  while ((opt = getopt(argc, argv, "di")) != -1) {
    switch (opt) {
    case 'd':
      dump = true;
      break;
    case 'i':
      use_ir = true;
      break;
    default:
      error("Usage: %s [-d] [-i] filename\n", argv[0]);
    }
  }

  char* path = format("%s/%s", dirname(format("%s", argv[optind])), basename(format("%s", argv[optind])));

  Token* tokens = lex(path);

  if (dump) {
    for (Token* t = tokens; t != NULL; t = t->next) {
      dump_token(t);
    }
    eprintf("\n");
  }

  tokens = preprocess(dirname(format("%s", path)), tokens);

  if (dump) {
    for (Token* t = tokens; t != NULL; t = t->next) {
      dump_token(t);
    }
    eprintf("\n");
  }

  Program* prog = parse(tokens);
  sema(prog);

  if (dump) {
    for (size_t i = 0; i < prog->funcs->length; i++) {
      dump_function(prog->funcs->ptr[i]);
    }
  }

  if (use_ir) {
    IProgram* iprog = gen_ir(prog);
    alloc_regs(iprog);
    eprintf("%s\n", show_iprog(iprog));
  } else {
    gen_x86(prog);
  }

  return 0;
}
