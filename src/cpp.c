#include "hoc.h"

typedef struct MacroEnv {
  struct MacroEnv* next;
  char* name;
  Token* tokens;
} MacroEnv;

static Token* input;
static Token* output;
static MacroEnv* gbl_env;

static Token* copy_token(Token* src) {
  Token* new = calloc(1, sizeof(Token));
  memcpy(new, src, sizeof(Token));
  new->next = NULL;
  return new;
}

static Token* copy_tokens(Token* src) {
  if (src == NULL) {
    return NULL;
  } else {
    Token* new = copy_token(src);
    new->next = copy_tokens(src->next);
    return new;
  }
}

static void add_objlike(char* name, Token* tokens, MacroEnv** prev) {
  MacroEnv* new = calloc(1, sizeof(MacroEnv));
  new->name = name;
  new->tokens = tokens;
  new->next = *prev;
  *prev = new;
}

static bool is_macro(Token* token) {
  if (token->tag == TIDENT) {
    for (MacroEnv* env = gbl_env; env != NULL; env = env->next) {
      if (streq(token->ident, env->name)) {
        return true;
      }
    }
  }
  return false;
}

static Token* get_macro_tokens(char* name) {
  for (MacroEnv* env = gbl_env; env != NULL; env = env->next) {
    if (streq(name, env->name)) {
      return copy_tokens(env->tokens);
    }
  }
  return false;
}

static void pp_error(char* expected, Token* actual) {
  bad_token(actual, format("%s expected", expected));
}

static void consume(void) {
  input = input->next;
}

static Token* expect(char* expected, enum TokenTag tag) {
  if (input->tag != tag) {
    pp_error(expected, input);
    return NULL;
  } else {
    Token* ret = input;
    consume();
    return ret;
  }
}

static void append(Token** dst, Token* src) {
  Token** p;
  for (p = dst; *p != NULL; p = &(*p)->next) {}
  *p = src;
}

static Token* read_until_bol(void) {
  if (input->bol) {
    return NULL;
  } else {
    Token* new = copy_token(input);
    consume();
    new->next = read_until_bol();
    return new;
  }
}

static void read_funclike_define(char* name) {
  error("undefined\n");
}

static void read_objlike_define(char* name) {
  add_objlike(name, read_until_bol(), &gbl_env);
}

static void read_define(char* name) {
  if (eq_reserved(input, "(")) {
    read_funclike_define(name);
  } else {
    read_objlike_define(name);
  }
}

static void apply_funclike(char* name) {

}

static void apply_objlike(char* name) {
  Token* ts = get_macro_tokens(name);
  append(&ts, input);
  input = ts;
}

static void apply(char* name) {
  if (eq_reserved(input, "(")) {
    apply_funclike(name);
  } else {
    apply_objlike(name);
  }
}

static void traverse(void) {
  if (input->tag == TDEFINE) {
    consume();
    read_define(expect("macro name", TIDENT)->ident);
    traverse();
  } else if (is_macro(input)) {
    char* name = expect("macro name", TIDENT)->ident;
    apply(name);
  } else {
    Token* new = copy_token(input);
    append(&output, new);
    consume();
  }
}

Token* preprocess(Token* tokens) {
  input = tokens;

  while (input) {
    traverse();
  }

  return output;
}
