#include "hoc.h"

int main(int argc, char** argv)
{
  bool dump = false;
  if (argc < 1) {
    error("./hoc file_name\n");
  }

  if (streq(argv[1], "-dump")) {
    dump = true;
  }


  Token* tokens;
  if (dump) {
    tokens = lex(argv[2]);
  } else {
    tokens = lex(argv[1]);
  }

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

  gen_x86(prog);

  return 0;
}
