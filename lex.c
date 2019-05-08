#include "hoc.h"

// ソースファイル
// Vector* lex(FILE* file);で初期化
static FILE* src;
static char c;

static void consume() {
  c = getc(src);
}

static void whitespace() {
  while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
    consume();
  }
}

static Token* new_token(enum TokenTag tag) {
  Token* t = malloc(sizeof(Token));
  t->tag = tag;
  return t;
}

static Token* integer() {
  Token* t = new_token(TINT);
  t->integer = c - '0';
  consume();
  while (isdigit(c)) {
    t->integer = t->integer * 10 + (c - '0');
    consume();
  }
  return t;
}

static Token* next_token() {
  while (c != EOF) {
    switch (c) {
    case ' ': case '\t': case '\n': case '\r':
      whitespace(); continue;
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
    case '8': case '9':
      return integer();
    case '+':
      consume();
      return new_token(TPLUS);
    default:
      error("invalid character: %c\n", c);
    }
  }
  return new_token(TEOF);
}

Vector* lex(FILE* file) {
  src = file;

  Vector* v = new_vec();
  Token* t;
  consume();

  while ((t = next_token())->tag != TEOF) {
    vec_push(v, t);
  }

  vec_push(v, t); // push EOF
  return v;
}
