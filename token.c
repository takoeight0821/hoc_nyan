#include "hoc.h"

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

static Token* new_token(enum TokenTag tag, char* start) {
  Token* t = calloc(1, sizeof(Token));
  t->tag = tag;
  t->start = start;
  return t;
}

static char* read_file(FILE* file) {
  StringBuilder* sb = new_sb();

  char c;
  while ((c = fgetc(file)) != EOF) {
    sb_putc(sb, c);
  }
  return sb_run(sb);
}

static void consume() {
  cur++;
}

static bool start_with(char* s1, char* s2) {
  while (*s1 != '\0' && *s2 != '\0') {
    if (*s1 == *s2) {
      s1++;
      s2++;
    } else {
      return false;
    }
  }
  return true;
}

static void whitespace() {
  while (*cur == ' ' || *cur == '\t' || *cur == '\n' || *cur == '\r') {
    consume();
  }
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
  while (isalnum(*cur) || *cur == '_') {
    sb_putc(sb, *cur);
    consume();
  }
  t->ident = sb_run(sb);
  return t;
}

struct Symbol {
  char* name;
  enum TokenTag tag;
};

struct Symbol symbols[] = {
  {"+", TPLUS},
  {"->", TARROW},
  {"-", TMINUS},
  {"*", TASTERISK},
  {"/", TSLASH},
  {"&&", TAND_AND},
  {"&", TAND},
  {"||", TOR_OR},
  {"%", TPERCENT},
  {"<=", TLE},
  {"<", TLT},
  {">=", TGE},
  {">", TGT},
  {"==", TEQ_EQ},
  {"=", TEQ},
  {"!=", TNE},
  {"!", TNOT},
  {"(", TLPAREN},
  {")", TRPAREN},
  {"{", TLBRACE},
  {"}", TRBRACE},
  {"[", TLBRACK},
  {"]", TRBRACK},
  {":", TCOLON},
  {";", TSEMICOLON},
  {",", TCOMMA},
  {".", TDOT},
};

static Token* next_token(void) {
  if (*cur == '\0') {
    return NULL; // new_token(TEOF, cur);
  }
  if (strchr(" \t\n\r", *cur)) {
    whitespace();
    return next_token();
  }
  if (strchr("0123456789", *cur)) {
    return integer(cur);
  }

  for (int i = 0; i < sizeof(symbols) / sizeof(struct Symbol); i++) {
    if (start_with(symbols[i].name, cur)) {
      char* start = cur;
      cur += strlen(symbols[i].name);
      return new_token(symbols[i].tag, start);
    }
  }

  if ('\"' == *cur) {
    char* start = cur;
    consume();
    StringBuilder* sb = new_sb();
    while (*(cur - 1) == '\\' || *cur != '\"') {
      sb_putc(sb, *cur);
      consume();
    }
    consume();
    Token* tok = new_token(TSTRING, start);
    tok->str = sb_run(sb);
    return tok;
  }

  if ('\'' == *cur) {
    consume();
    Token* t = new_token(TINT, cur - 1);
    if (*cur == '\\') {
      consume();
      switch (*cur) {
      case 'a':
        t->integer = '\a';
        break;
      case 'b':
        t->integer = '\b';
        break;
      case 'f':
        t->integer = '\f';
        break;
      case 'n':
        t->integer = '\n';
        break;
      case 'r':
        t->integer = '\r';
        break;
      case 't':
        t->integer = '\t';
        break;
      case 'v':
        t->integer = '\v';
        break;
      case '\\':
        t->integer = '\\';
        break;
      case '\'':
        t->integer = '\'';
        break;
      case '\"':
        t->integer = '\"';
        break;
      default:
        print_line(cur);
        error("invalid escape sequence: %c\n", *cur);
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

  if (isalpha(*cur) || *cur == '_') {
    return ident(cur);
  }

  print_line(cur);
  error("invalid character: %c\n", *cur);
}

Token* lex(char* path) {
  FILE* file = fopen(path, "r");
  if (file == NULL) {
    error("cannot open file\n");
  }

  src = read_file(file);
  fclose(file);
  cur = src;

  Token* current = next_token();
  Token* head = current;

  while ((current->next = next_token()) != NULL) {
    current = current->next;
  }

  return head;
}
