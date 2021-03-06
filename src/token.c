#include "hoc.h"

// ソースコードの先頭文字
static char* src;
// トークナイザの現在位置
static char* cur;
// 行頭か否か
static bool bol = 1;

static void print_line(char* start, char* pos) {
  size_t line = 0;
  size_t column = 0;
  /* char* start = src; // 出力する行の先頭文字 */

  for (char *c = start; c != pos && *c != '\0'; c++) {
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
  for (size_t i = 0; i < column; i++) {
    if (start[i] == '\t') {
      eprintf("\t");
    } else {
      eprintf(" ");
    }
  }

  eprintf("^\n\n");
}

void warn_token(Token* tok, char* msg) {
  print_line(tok->source, tok->start);
  eprintf("%s\n", msg);
}

void bad_token(Token* tok, char* msg) {
  warn_token(tok, msg);
  exit(1);
}

static Token* new_token(enum TokenTag tag, char* start) {
  Token* t = calloc(1, sizeof(Token));
  t->tag = tag;
  t->source = src;
  t->start = start;
  t->bol = bol;
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
  if (*cur == '\n' || *cur == '\r') {
    bol = true;
  } else if (!(*cur == ' ' || *cur == '\t')) {
    bol = false;
  }
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

static char read_char(void) {
  char c;
  if (*cur == '\\') {
    consume();
    switch (*cur) {
    case 'a':
      c = '\a';
      break;
    case 'b':
      c = '\b';
      break;
    case 'f':
      c = '\f';
      break;
    case 'n':
      c = '\n';
      break;
    case 'r':
      c = '\r';
      break;
    case 't':
      c = '\t';
      break;
    case 'v':
      c = '\v';
      break;
    case '\\':
      c = '\\';
      break;
    case '\'':
      c = '\'';
      break;
    case '\"':
      c = '\"';
      break;
    case '0':
      c = '\0';
      break;
    default:
      print_line(src, cur);
      error("invalid escape sequence: %c\n", *cur);
    }
  } else {
    c = *cur;
  }
  consume();
  return c;
}

static void whitespace() {
  while (*cur == ' ' || *cur == '\t' || *cur == '\n' || *cur == '\r') {
    consume();
  }
}

static char* punctuators[46] = {
  "...", "<<=", ">>=",
  "->", "++", "--", "<<", ">>", "<=", ">=", "==", "!=",
  "&&", "||", "*=", "/=", "%=", "+=", "-=", "&=", "^=",
  "|=",
  "[", "]", "(", ")", "{", "}", ".", "&", "*", "+",
  "-", "~", "!", "/", "%", "<", ">", "^", "|", "?",
  ":", ";", "=", ","
};

static char* keywords[43] = {
  "auto", "break", "case", "char", "const", "continue", "default",
  "do", "double", "else", "enum", "extern", "float", "for", "goto",
  "if", "inline", "int", "long", "register", "restrict", "return",
  "short", "sizeof", "static", "struct", "switch", "typedef",
  "union", "unsigned", "void", "volatile", "while",
  "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic",
  "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"
};

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

static Token* new_reserved(char* start, char* ident) {
  Token* t = new_token(TRESERVED, start);
  t->ident = ident;
  return t;
}

static char* read_include_path(void) {
  StringBuilder* sb = new_sb();

  if (*cur != '"' && *cur != '<') {
    print_line(src, cur);
    error("expected \" or <, but actual %c\n", *cur);
  }
  consume();

  while (*cur != '"' && *cur != '>') {
    sb_putc(sb, *cur);
    consume();
  }

  consume();
  return sb_run(sb);
}

static Token* next_token(void) {
  if (*cur == '\0') {
    return NULL;
  }

  if (start_with("/*", cur)) {
    while (!start_with("*/", cur)) {
      consume();
    }
    consume();
    consume();
    return next_token();
  }

  if (start_with("//", cur)) {
    consume();
    consume();
    while (*cur != '\n') {
      consume();
    }
    return next_token();
  }

  if (bol && *cur == '#') {
    consume();
    if (start_with("define", cur)) {
      Token* token = new_token(TDIRECTIVE, cur);
      token->bol = true;
      cur += strlen("define");
      token->ident = "define";
      return token;
    } else if (start_with("include", cur)) {
      Token* token = new_token(TDIRECTIVE, cur);
      token->bol = true;
      cur += strlen("include");
      token->ident = "include";
      whitespace();
      token->str = read_include_path();
      return token;
    } else if (start_with("ifdef", cur)) {
      Token* token = new_token(TDIRECTIVE, cur);
      token->bol = true;
      cur += strlen("ifdef");
      token->ident = "ifdef";
      return token;
    } else if (start_with("ifndef", cur)) {
      Token* token = new_token(TDIRECTIVE, cur);
      token->bol = true;
      cur += strlen("ifndef");
      token->ident = "ifndef";
      return token;
    } else if (start_with("endif", cur)) {
      Token* token = new_token(TDIRECTIVE, cur);
      token->bol = true;
      cur += strlen("endif");
      token->ident = "endif";
      return token;
    } else {
      print_line(src, cur);
      error("invalid preprocessing directive\n");
    }
  } else if (*cur == '#') {
    print_line(src, cur);
    error("invalid #\n");
  }

  if (strchr(" \t\n\r", *cur)) {
    whitespace();
    return next_token();
  }

  if (strchr("0123456789", *cur)) {
    return integer(cur);
  }

  for (int i = 0; i < (sizeof(punctuators) / sizeof(char*)); i++) {
    if (start_with(punctuators[i], cur)) {
      Token* t = new_reserved(cur, punctuators[i]);
      cur += strlen(punctuators[i]);
      return t;
    }
  }

  for (int i = 0; i < (sizeof(keywords) / sizeof(char*)); i++) {
    if (start_with(keywords[i], cur)) {
      Token* t = new_reserved(cur, keywords[i]);
      cur += strlen(keywords[i]);
      if (isalnum(*cur) || *cur == '_') {
        cur -= strlen(keywords[i]);
      } else {
        return t;
      }
    }
  }

  if ('\"' == *cur) {
    char* start = cur;
    consume();
    StringBuilder* sb = new_sb();
    while (*cur != '\"') {
      sb_putc(sb, read_char());
    }
    consume();
    Token* tok = new_token(TSTRING, start);
    tok->str = sb_run(sb);
    return tok;
  }

  if ('\'' == *cur) {
    consume();
    Token* t = new_token(TINT, cur - 1);
    t->integer = read_char();
    if (*cur != '\'') {
      print_line(src, cur);
      error("expected ' but actual %c\n", *cur);
    }
    consume();
    return t;
  }

  if (isalpha(*cur) || *cur == '_') {
    return ident(cur);
  }

  print_line(src, cur);
  error("invalid character: %c\n", *cur);
}

Token* lex(char* path) {
  FILE* file = fopen(path, "r");
  if (file == NULL) {
    error("cannot open file: %s\n", path);
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
