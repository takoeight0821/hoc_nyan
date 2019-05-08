#include "hoc.h"
#include <stdlib.h>
#include <ctype.h>

FILE* fp;
long prev; // 直前にreadcしたファイル位置

void dump_token(Token tok) {
  switch (tok.tag) {
  case NINT:
    printf("[INT %d]", tok.integer);
    break;
  case NPLUS:
    printf("[PLUS]");
    break;
  }
}

char readc() {
  prev = ftell(fp);
  return fgetc(fp);
}

void back() {
  fseek(fp, prev, SEEK_SET);
}

Vector *text;

void addText(char c) {
  char* ch = malloc(sizeof(char));
  *ch = c;
  vec_push(text, ch);
}

char* to_string(Vector* v) {
  char* str = calloc(v->length+1, sizeof(char));
  size_t len = v->length;
  for (int i = 0; i < len; i++) {
    char c = *(char*)vec_pop(v);
    str[i] = c;
  }
  str[len] = '\0';
  return str;
}

Token* new_token(enum TokenTag tag) {
  Token* tok = malloc(sizeof(Token));
  tok->tag = tag;
  return tok;
}

Vector* lex(FILE* file) {
  char c;
  fp = file;
  text = new_vec();
  Vector* tokens = new_vec();

  while ((c = readc()) != EOF) {
    switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      addText(c);
      while (isdigit(c = readc())) {
        addText(c);
      }
      back();
      Token* t = new_token(TINT);
      t->integer = atoi(to_string(text));
      vec_push(tokens, t);
      break;
    }
    case '+': {
      Token* t = new_token(TPLUS);
      vec_push(tokens, t);
      break;
    }
    }
  }
  return tokens;
}
