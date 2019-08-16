#include <hoc.h>

int main(int argc, char** argv)
{
  bool dump = false;
  bool use_ir = false;
  if (argc < 1) {
    error("./hoc file_name\n");
  }

  if (streq(argv[1], "-dump")) {
    dump = true;
  }

  if (streq(argv[1], "-ir")) {
    use_ir = true;
  }


  Token* tokens;
  if (dump || use_ir) {
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

  tokens = preprocess(tokens);

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
    eprintf("%s\n", show_iprog(iprog));
  }
  gen_x86(prog);

  return 0;
}
