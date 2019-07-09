#include "hoc.h"

// ソースコードの最大文字数
#define MAX_LENGTH 4096

// ソースコード
// Vector* lex(FILE* file);で初期化
char* src;
char* cur;

void print_line(char* pos) {
  size_t line = 0;
  size_t column = 0;
  char* start = src; // 出力する行の先頭文字

  for (char *c = src; c != pos; c++) {
    if (*c == '\n') {
      start = c + 1;
      line++;
      column = 0;
    } else {
      column++;
    }
  }

  eprintf("error at (%d, %d)\n\n", line + 1, column + 1);

  // 行を出力
  size_t len = strchr(start, '\n') - start;
  eprintf("%.*s\n", len, start);

  // posまでインデントする
  for (size_t i = 0; i < column; i++)
    eprintf("%c", (start[i] == '\t') ? '\t' : ' ');

  eprintf("^\n\n");
}

void warn_token(Token* tok, char* msg) {
  print_line(tok->start);
  eprintf("%s\n", msg);
}

noreturn void bad_token(Token* tok, char* msg) {
  warn_token(tok, msg);
  exit(1);
}

static void consume() {
  cur++;
}

static void whitespace() {
  while (*cur == ' ' || *cur == '\t' || *cur == '\n' || *cur == '\r') {
    consume();
  }
}

static Token* new_token(enum TokenTag tag, char* start) {
  Token* t = calloc(1, sizeof(Token));
  t->tag = tag;
  t->start = start;
  return t;
}

static Token* integer(char* start) {
  Token* t = new_token(TINT, start);
  t->integer = 0;
  while (isdigit(*cur)) {
    t->integer = t->integer * 10 + (*cur - '0');
    consume();
  }
  return t;
}

static Token* ident(char* start) {
  Token* t = new_token(TIDENT, start);
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
      return integer(cur);
    case '+':
      consume();
      return new_token(TPLUS, cur - 1);
    case '-':
      consume();
      return new_token(TMINUS, cur - 1);
    case '*':
      consume();
      return new_token(TASTERISK, cur - 1);
    case '/':
      consume();
      return new_token(TSLASH, cur - 1);
    case '&':
      consume();
      return new_token(TAND, cur - 1);
    case '<':
      consume();
      switch (*cur) {
      case '=':
        consume();
        return new_token(TLE, cur - 2);
      default:
        return new_token(TLT, cur - 1);
      }
    case '>':
      consume();
      switch (*cur) {
      case '=':
        consume();
        return new_token(TGE, cur - 2);
      default:
        return new_token(TGT, cur - 1);
      }
    case '=':
      consume();
      switch (*cur) {
      case '=':
        consume();
        return new_token(TEQ, cur - 2);
      default:
        return new_token(TEQUAL, cur - 1);
      }
    case '!':
      consume();
      switch (*cur) {
      case '=':
        consume();
        return new_token(TNE, cur - 2);
      default:
        // TODO `not` operator
        print_line(cur);
        error("invalid character: %c\n", *cur);
      }
    case '(':
      consume();
      return new_token(TLPAREN, cur - 1);
    case ')':
      consume();
      return new_token(TRPAREN, cur - 1);
    case '{':
      consume();
      return new_token(TLBRACE, cur - 1);
    case '}':
      consume();
      return new_token(TRBRACE, cur - 1);
    case '[':
      consume();
      return new_token(TLBRACK, cur - 1);
    case ']':
      consume();
      return new_token(TRBRACK, cur - 1);
    case ';':
      consume();
      return new_token(TSEMICOLON, cur - 1);
    case ',':
      consume();
      return new_token(TCOMMA, cur - 1);
    case '\"': {
      char* start = cur;
      consume();
      Vector* str = new_vec();
      while(*cur != '\"' || *cur == '\0') {
        vec_pushi(str, *cur);
        consume();
      }
      if (*cur == '\0') {
        print_line(cur);
        error("invalid character: %c\n", *cur);
      }
      consume();
      Token* tok = new_token(TSTRING, start);
      tok->str = vec_to_string(str);
      return tok;
    }
    default:
      if (isalpha(*cur) || *cur == '_') {
        return ident(cur);
      }
      print_line(cur);
      error("invalid character: %c\n", *cur);
    }
  }
  return new_token(TEOF, cur);
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

void dump_token(Token* tok) {
  switch (tok->tag) {
  case TINT:
    eprintf("[INT %d]", tok->integer);
    break;
  case TIDENT:
    eprintf("[IDENT %s]", tok->ident);
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
  case TAND:
    eprintf("[AND]");
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
  case TLBRACE:
    eprintf("[LBRACE]");
    break;
  case TRBRACE:
    eprintf("[RBRACE]");
    break;
  case TLBRACK:
    eprintf("[LBRACK]");
    break;
  case TRBRACK:
    eprintf("[RBRACK]");
    break;
  case TSEMICOLON:
    eprintf("[SEMICOLON]");
    break;
  case TCOMMA:
    eprintf("[COMMA]");
    break;
  case TSTRING:
    eprintf("[STRING %s]", tok->str);
    break;
  case TEOF:
    eprintf("[EOF]");
    break;
  }
}
