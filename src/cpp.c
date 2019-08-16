#include <hoc.h>

typedef struct MacroEnv {
  struct MacroEnv* next;
  char* name;
  Token* tokens;
  Vector* params;
} MacroEnv;

static Token* input;
static Token* output;
static MacroEnv* gbl_env;

static void traverse(void);

static Token* copy_token(Token* src) {
  Token* new = calloc(1, sizeof(Token));
  memcpy(new, src, sizeof(Token));
  /* dump_token(new); */
  /* warn_token(new, "DEBUG copy_token"); */
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

static void add_funclike(char* name, Token* tokens, Vector* params, MacroEnv** prev) {
  MacroEnv* new = calloc(1, sizeof(MacroEnv));
  new->name = name;
  new->tokens = tokens;
  new->params = params;
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

static MacroEnv* get_local_env(char* name, Vector* args) {
  Vector* params;
  for (MacroEnv* env = gbl_env; env != NULL; env = env->next) {
    if (streq(name, env->name)) {
      params = env->params;
    }
  }

  MacroEnv* lcl_env = NULL;
  MacroEnv** p = &lcl_env;
  for (int i = 0; i < params->length; i++) {
    MacroEnv* new = calloc(1, sizeof(MacroEnv));
    new->name = params->ptr[i];
    new->tokens = args->ptr[i];
    new->next = *p;
    *p = new;
  }

  return lcl_env;
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
  for (p = dst; *p != NULL; p = &(*p)->next) {
  }
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

static Vector* read_funclike_params(void) {
  Vector* params = new_vec();
  while (!eq_reserved(input, ")")) {
    char* name = expect("macro parameter", TIDENT)->ident;
    vec_push(params, name);
    if (eq_reserved(input, ",")) {
      consume();
    } else if (!eq_reserved(input, ")")) {
      pp_error(") or ,", input);
    }
  }
  consume();
  return params;
}

static void read_funclike_define(char* name) {
  Vector* params = read_funclike_params();
  Token* tokens = read_until_bol();
  add_funclike(name, tokens, params, &gbl_env);
}

static void read_objlike_define(char* name) {
  add_objlike(name, read_until_bol(), &gbl_env);
}

static void read_define(char* name) {
  if (eq_reserved(input, "(")) {
    consume();
    read_funclike_define(name);
  } else {
    read_objlike_define(name);
  }
}

static Token* read_one_arg(void) {
  Token* arg = NULL;
  while (!eq_reserved(input, ")") && !eq_reserved(input, ",")) {
    append(&arg, copy_token(input));
    consume();
  }
  return arg;
}

static Vector* read_funclike_args(void) {
  Vector* args = new_vec();
  while (!eq_reserved(input, ")")) {
    Token* arg = read_one_arg();
    vec_push(args, arg);
    if (eq_reserved(input, ",")) {
      consume();
    } else if (!eq_reserved(input, ")")) {
      pp_error(") or ,", input);
    }
  }
  consume();
  return args;
}

static void apply_funclike(char* name) {
  Vector* args = read_funclike_args();
  MacroEnv* lcl_env = get_local_env(name, args);
  Token* func_tokens = get_macro_tokens(name);

  MacroEnv* env_backup = gbl_env;
  Token* input_backup = input;
  Token* output_backup = output;

  gbl_env = lcl_env;
  input = func_tokens;
  output = NULL;

  while (input) {
    traverse();
  }

  Token* expanded = output;

  gbl_env = env_backup;
  input = input_backup;
  output = output_backup;

  append(&expanded, input);
  input = expanded;
}

static void apply_objlike(char* name) {
  Token* ts = get_macro_tokens(name);

  append(&ts, input);
  input = ts;
}

static void apply(char* name) {
  if (input && eq_reserved(input, "(")) {
    consume();
    apply_funclike(name);
  } else {
    apply_objlike(name);
  }
}

static char* include_path(char* path) {
  return format("./include/%s", path);
}

static void skip_to_endif(void) {
  while (input && !(input->tag == TDIRECTIVE && streq(input->ident, "endif"))) {
    consume();
  }
  consume();
}

static void traverse(void) {
  if (input->tag == TDIRECTIVE && streq(input->ident, "define")) {
    consume();
    read_define(expect("macro name", TIDENT)->ident);
    traverse();
  } else if (input->tag == TDIRECTIVE && streq(input->ident, "include")) {
    Token* included = lex(include_path(input->str));
    append(&included, input->next);
    input = included;
  } else if (input->tag == TDIRECTIVE && streq(input->ident, "ifdef")) {
    consume();
    Token* tag = expect("macro name", TIDENT);
    if (!is_macro(tag)) {
      skip_to_endif();
    }
  } else if (input->tag == TDIRECTIVE && streq(input->ident, "ifndef")) {
    consume();
    Token* tag = expect("macro name", TIDENT);
    if (is_macro(tag)) {
      skip_to_endif();
    }
  } else if (input->tag == TDIRECTIVE && streq(input->ident, "endif")) {
    consume();
  } else if (is_macro(input)) {
    // TODO: 関数マクロの仮引数名と実引数が同じだと無限ループする
    char* name = expect("macro name", TIDENT)->ident;
    apply(name);
  } else {
    Token* new = copy_token(input);
    append(&output, new);
    consume();
  }
}

Token* preprocess(Token* tokens) {
  gbl_env = calloc(1, sizeof(MacroEnv));
  gbl_env->name = "__hoc__";
  input = tokens;

  while (input) {
    traverse();
  }

  return output;
}
