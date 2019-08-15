#include "hoc.h"

typedef struct MacroEnv {
  struct MacroEnv* next;
  char* name;
  Token* tokens;
} MacroEnv;

static MacroEnv* macro_env;

static void preprocess_error(char* expected, Token* actual) {
  bad_token(actual, format("%s expected", expected));
}

static bool is_macro(Token* token) {
  if (token->tag == TIDENT) {
    for (MacroEnv* env = macro_env; env != NULL; env = env->next) {
      if (streq(token->ident, env->name)) {
        return true;
      }
    }
  }
  return false;
}

static Token* append(Token* t1, Token* t2) {
  if (t1 == NULL) {
    return t2;
  } else {
    t1->next = append(t1->next, t2);
    return t1;
  }
}

static Token* apply(char* name, Token* input) {
  Token* macro;
  for (MacroEnv* env = macro_env; env != NULL; env = env->next) {
    if (streq(name, env->name)) {
      macro = env->tokens;
    }
  }

  return append(macro, input);
}

static Token* read_until_bol(char* macro_name, Token* token) {
  if (token->bol) {
    return token;
  } else {
    for (MacroEnv* env = macro_env; env != NULL; env = env->next) {
      if (streq(macro_name, env->name)) {
        Token* new_token = calloc(1, sizeof(Token));
        memcpy(new_token, token, sizeof(Token));
        new_token->next = NULL;
        env->tokens = append(env->tokens, new_token);
        return read_until_bol(macro_name, token->next);
      }
    }
    preprocess_error("macro definition", token);
  }
  return NULL;
}

static Token* read_define(Token* token) {
  if (token->tag != TIDENT) {
    preprocess_error("ident", token);
  }

  MacroEnv* new_macro = calloc(1, sizeof(MacroEnv));
  new_macro->name = token->ident;
  new_macro->next = macro_env;
  macro_env = new_macro;

  return read_until_bol(macro_env->name, token->next);
}

static Token* traverse(Token* input) {
  if (!input) {
    return NULL;
  }

  if (input->tag == TDEFINE) {
    input = read_define(input->next);
  }

  if (is_macro(input)) {
    return traverse(apply(input->ident, input->next));
  }

  input->next = traverse(input->next);
  return input;
}

Token* preprocess(Token* tokens) {
  return traverse(tokens);
}
