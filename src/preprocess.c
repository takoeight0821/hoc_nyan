#include "hoc.h"

typedef struct MacroEnv {
  struct MacroEnv* next;
  char* name;
  Vector* params;
  Token* tokens;
} MacroEnv;

static MacroEnv* macro_env;

static Token* traverse(Token* input);

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

static MacroEnv* find_macro(char* name) {
  for (MacroEnv* env = macro_env; env != NULL; env = env->next) {
    if (streq(name, env->name)) {
      return env;
    }
  }
  error("macro %s is not defined\n", name);
}

static Token* append(Token* t1, Token* t2) {
  if (t1 == NULL) {
    return t2;
  } else {
    t1->next = append(t1->next, t2);
    return t1;
  }
}

static Token* apply_objlike(char* name, Token* input) {
  Token* macro = find_macro(name)->tokens;
  return append(macro, input);
}

static MacroEnv* make_funcenv(Vector* params, Vector* args) {
  MacroEnv* env = NULL;

  for (int i = 0; i < params->length; i++) {
    MacroEnv* new = calloc(1, sizeof(MacroEnv));
    new->name = params->ptr[i];
    new->tokens = args->ptr[i];
    new->next = env;
    env = new;
  }

  return env;
}

static Token* apply_funclike(char* name, Vector* args, Token* rest) {
  MacroEnv* funcenv = make_funcenv(find_macro(name)->params, args);
  MacroEnv* globalenv = macro_env;
  Token* macro_tokens = find_macro(name)->tokens;
  macro_env = funcenv;

  Token* evaled = traverse(macro_tokens);

  macro_env = globalenv;

  return append(evaled, rest);
}

static Token* read_funclike_arguments(Vector* args, Token* input) {
  // 1引数のみサポート

  Token* arg = calloc(1, sizeof(Token));
  Token* cur = arg;
  while (!eq_reserved(input, ")")) {
    memcpy(cur, input, sizeof(Token));
    input = input->next;
    if (!eq_reserved(input, ")")) {
      cur->next = calloc(1, sizeof(Token));
      cur = cur->next;
    } else {
      cur->next = NULL;
    }
  }
  vec_push(args, arg);

  if (!eq_reserved(input, ")")) {
    preprocess_error(")", input);
  }
  return input->next;
}

static Token* apply(char* name, Token* input) {
  if (input && eq_reserved(input, "(")) {
    Vector* args = new_vec();
    input = read_funclike_arguments(args, input->next);
    return apply_funclike(name, args, input);
  } else {
    return apply_objlike(name, input);
  }
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

static Token* read_define_objlike(char* name, Token* token) {

  MacroEnv* new_macro = calloc(1, sizeof(MacroEnv));
  new_macro->name = name;
  new_macro->next = macro_env;
  macro_env = new_macro;

  return read_until_bol(macro_env->name, token);
}

static Token* read_funclike_parameter(char* macro_name, Vector* params, Token* token) {
  while (!eq_reserved(token, ")")) {
    if (token->tag != TIDENT) {
      preprocess_error("macro parameter", token);
    }
    char* param = token->ident;
    vec_push(params, param);
    token = token->next;
    if (eq_reserved(token, ")")) {
      break;
    }
    if (!eq_reserved(token, ",")) {
      preprocess_error(", or )", token);
    }
  }
  return token->next;
}

static Token* read_define_funclike(char* macro_name, Token* token) {
  MacroEnv* new_macro = calloc(1, sizeof(MacroEnv));
  new_macro->name = macro_name;
  new_macro->next = macro_env;
  macro_env = new_macro;

  if (!eq_reserved(token, "(")) {
    preprocess_error("(", token);
  }

  new_macro->params = new_vec();
  token = read_funclike_parameter(new_macro->name, new_macro->params, token->next);

  token = read_until_bol(macro_env->name, token);
  return token;
}

static Token* traverse(Token* input) {
  if (!input) {
    return NULL;
  }

  if (input->tag == TDEFINE) {
    if (input->next && input->next->tag != TIDENT) {
      preprocess_error("macro name", input->next);
    }
    input = input->next;


    if (input->next && eq_reserved(input->next, "(")) {
      return traverse(read_define_funclike(input->ident, input->next));
    } else {
      return traverse(read_define_objlike(input->ident, input->next));
    }
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
