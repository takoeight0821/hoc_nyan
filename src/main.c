#include "hoc.h"

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
  char* path;
  if (dump || use_ir) {
    path = format("%s/%s", dirname(format("%s", argv[2])), basename(format("%s", argv[2])));
  } else {
    path = format("%s/%s", dirname(format("%s", argv[1])), basename(format("%s", argv[1])));
  }

  tokens = lex(path);

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
    eprintf("%s\n", show_iprog(iprog));
  }
  gen_x86(prog);

  return 0;
}
