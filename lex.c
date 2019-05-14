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
  t->integer = 0;
  while (isdigit(c)) {
    t->integer = t->integer * 10 + (c - '0');
    consume();
  }
  return t;
}

static Token* ident() {
  Token* t = new_token(TIDENT);
  Vector* ident = new_vec();
  vec_pushi(ident, c);
  consume();

  while (isalnum(c) || c == '_') {
    vec_pushi(ident, c);
    consume();
  }

  t->ident = vec_to_string(ident);

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
    case '-':
      consume();
      return new_token(TMINUS);
    case '*':
      consume();
      return new_token(TASTERISK);
    case '/':
      consume();
      return new_token(TSLASH);
    case '<':
      consume();
      switch (c) {
      case '=':
        consume();
        return new_token(TLE);
      default:
        return new_token(TLT);
      }
    case '>':
      consume();
      switch (c) {
      case '=':
        consume();
        return new_token(TLE);
      default:
        return new_token(TLT);
      }
    case '=':
      consume();
      switch (c) {
      case '=':
        consume();
        return new_token(TEQ);
      default:
        return new_token(TEQUAL);
      }
    case '!':
      consume();
      switch (c) {
      case '=':
        consume();
        return new_token(TNE);
      default:
        // TODO `not` operator
        error("invalid character: %c\n", c);
      }
    case '(':
      consume();
      return new_token(TLPAREN);
    case ')':
      consume();
      return new_token(TRPAREN);
    case ';':
      consume();
      return new_token(TSEMICOLON);
    default:
      if (isalpha(c) || c == '_') {
        return ident();
      }
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

void dump_token(Token tok) {
  switch (tok.tag) {
  case TINT:
    eprintf("[INT %d]", tok.integer);
    break;
  case TIDENT:
    eprintf("[IDENT %s]", tok.ident);
    break;
  case TLE:
    eprintf("[LE]");
    break;
  case TLT:
    eprintf("[LT]");
    break;
  case TGE:
    eprintf("[GE]");
    break;
  case TGT:
    eprintf("[GT]");
    break;
  case TEQ:
    eprintf("[EQ]");
    break;
  case TNE:
    eprintf("[NE]");
    break;
  case TPLUS:
    eprintf("[PLUS]");
    break;
  case TMINUS:
    eprintf("[MINUS]");
    break;
  case TASTERISK:
    eprintf("[ASTERISK]");
    break;
  case TSLASH:
    eprintf("[SLASH]");
    break;
  case TEQUAL:
    eprintf("[EQUAL]");
    break;
  case TLPAREN:
    eprintf("[LPAREN]");
    break;
  case TRPAREN:
    eprintf("[RPAREN]");
    break;
  case TSEMICOLON:
    eprintf("[SEMICOLON]");
    break;
  case TEOF:
    eprintf("[EOF]");
    break;
  }
}
