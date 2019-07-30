#include "hoc.h"

// ソースコードの最大文字数
#define MAX_LENGTH 4096

// ソースコード
static char* src;
static char* cur;

static void print_line(char* pos) {
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

void bad_token(Token* tok, char* msg) {
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
  StringBuilder* sb = new_sb();
  sb_putc(sb, *cur);
  consume();

  while (isalnum(*cur) || *cur == '_') {
    sb_putc(sb, *cur);
    consume();
  }

  t->ident = sb_run(sb);

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
      switch(*cur) {
      case '+':
        consume();
        return new_token(TPLUS_PLUS, cur - 2);
      default:
        return new_token(TPLUS, cur - 1);
      }
    case '-':
      consume();
      switch(*cur) {
      case '>':
        consume();
        return new_token(TARROW, cur - 2);
      default:
        return new_token(TMINUS, cur - 1);
      }
    case '*':
      consume();
      return new_token(TASTERISK, cur - 1);
    case '/':
      consume();
      return new_token(TSLASH, cur - 1);
    case '&':
      consume();
      switch (*cur) {
      case '&':
        consume();
        return new_token(TAND_AND, cur - 2);
      default:
        return new_token(TAND, cur - 1);
      }
    case '|':
      consume();
      switch (*cur) {
      case '|':
        consume();
        return new_token(TOR_OR, cur - 2);
      default:
        print_line(cur);
        error("invalid character: %c\n", *cur);
      }
    case '%':
      consume();
      return new_token(TPERCENT, cur - 1);
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
        return new_token(TNOT, cur - 1);
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
      StringBuilder* sb = new_sb();
      while(*cur != '\"' || *cur == '\0') {
        sb_putc(sb, *cur);
        consume();
      }
      if (*cur == '\0') {
        print_line(cur);
        error("invalid character: %c\n", *cur);
      }
      consume();
      Token* tok = new_token(TSTRING, start);
      tok->str = sb_run(sb);
      return tok;
    }
    case '\'': {
      consume();
      Token* t = new_token(TINT, cur - 1);
      if (*cur == '\\') {
        consume();
        switch (*cur) {
        case 'n':
          t->integer = '\n';
          break;
        case 't':
          t->integer = '\t';
          break;
        default:
          print_line(cur);
          error("invalid character: %c\n", *cur);
        }
      } else {
        t->integer = *cur;
      }
      consume();
      if (*cur != '\'') {
        print_line(cur);
        error("invalid character: %c\n", *cur);
      }
      consume();
      return t;
    }
    case '.':
      consume();
      return new_token(TDOT, cur - 1);
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
  StringBuilder* sb = new_sb();

  char c;
  while ((c = fgetc(file)) != EOF) {
    sb_putc(sb, c);
  }
  return sb_run(sb);
}

Token* lex(FILE* file) {
  src = read_file(file);
  cur = src;

  Token* current = NULL;
  Token* head;
  Token* t;

  while ((t = next_token())->tag != TEOF) {
    if (current) {
      current->next = t;
      current = current->next;
    } else {
      current = t;
      head = current;
    }
  }

  return head;
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
  case TDOT:
    eprintf("[DOT]");
    break;
  case TNOT:
    eprintf("[NOT]");
    break;
  case TAND_AND:
    eprintf("[AND_AND]");
    break;
  case TOR_OR:
    eprintf("[OR_OR]");
    break;
  case TPERCENT:
    eprintf("[PERCENT]");
    break;
  case TARROW:
    eprintf("[ARROW]");
    break;
  case TPLUS_PLUS:
    eprintf("[PLUS_PLUS]");
    break;
  case TEOF:
    eprintf("[EOF]");
    break;
  }
}
