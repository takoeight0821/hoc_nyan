#include "hoc.h"

// ソースコード
// Vector* lex(FILE* file);で初期化
/* static FILE* src; */
#define MAX_LENGTH 4096
char* src;
char* cur;

static void consume() {
  cur++;
}

static void whitespace() {
  while (*cur == ' ' || *cur == '\t' || *cur == '\n' || *cur == '\r') {
    consume();
  }
}

static Token* new_token(enum TokenTag tag) {
  Token* t = calloc(1, sizeof(Token));
  t->tag = tag;
  return t;
}

static Token* integer() {
  Token* t = new_token(TINT);
  t->integer = 0;
  while (isdigit(*cur)) {
    t->integer = t->integer * 10 + (*cur - '0');
    consume();
  }
  return t;
}

static Token* ident() {
  Token* t = new_token(TIDENT);
  Vector* ident = new_vec();
  vec_pushi(ident, *cur);
  consume();

  while (isalnum(*cur) || *cur == '_') {
    vec_pushi(ident, *cur);
    consume();
  }

  t->ident = vec_to_string(ident);

  return t;
}

static Token* next_token() {
  while (*cur != '\0') {
    switch (*cur) {
    case ' ': case '\t': case '\n': case '\r':
      whitespace(); continue;
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
    case '8': case '9':
      return integer();
    case '+':
      consume();
      return new_token(TPLUS);
    case '-':
      consume();
      return new_token(TMINUS);
    case '*':
      consume();
      return new_token(TASTERISK);
    case '/':
      consume();
      return new_token(TSLASH);
    case '&':
      consume();
      return new_token(TAND);
    case '<':
      consume();
      switch (*cur) {
      case '=':
        consume();
        return new_token(TLE);
      default:
        return new_token(TLT);
      }
    case '>':
      consume();
      switch (*cur) {
      case '=':
        consume();
        return new_token(TGE);
      default:
        return new_token(TGT);
      }
    case '=':
      consume();
      switch (*cur) {
      case '=':
        consume();
        return new_token(TEQ);
      default:
        return new_token(TEQUAL);
      }
    case '!':
      consume();
      switch (*cur) {
      case '=':
        consume();
        return new_token(TNE);
      default:
        // TODO `not` operator
        error("invalid character: %c\n", *cur);
      }
    case '(':
      consume();
      return new_token(TLPAREN);
    case ')':
      consume();
      return new_token(TRPAREN);
    case '{':
      consume();
      return new_token(TLBRACE);
    case '}':
      consume();
      return new_token(TRBRACE);
    case '[':
      consume();
      return new_token(TLBRACK);
    case ']':
      consume();
      return new_token(TRBRACK);
    case ';':
      consume();
      return new_token(TSEMICOLON);
    case ',':
      consume();
      return new_token(TCOMMA);
    default:
      if (isalpha(*cur) || *cur == '_') {
        return ident();
      }
      error("invalid character: %c\n", *cur);
    }
  }
  return new_token(TEOF);
}

static char* read_file(FILE* file) {
  Vector* buf = new_vec();

  char c;
  while ((c = fgetc(file)) != EOF) {
    vec_pushi(buf, c);
  }
  vec_pushi(buf, 0);
  return vec_to_string(buf);
}

Vector* lex(FILE* file) {
  src = read_file(file);
  cur = src;

  Vector* v = new_vec();
  Token* t;

  while ((t = next_token())->tag != TEOF) {
    vec_push(v, t);
  }

  vec_push(v, t); // push EOF
  return v;
}
