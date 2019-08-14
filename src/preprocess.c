#include "hoc.h"

/* struct MacroEnv { */
/*   struct MacroEnv* next; */
/*   char* name; */
/*   Token* tokens; */
/* }; */

static Token* emit(Token* token) {
  if (token) {
    token->next = emit(token->next);
    return token;
  } else {
    return NULL;
  }
}

Token* preprocess(Token* tokens) {
  return emit(tokens);
}
