#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  void **ptr;
  size_t capacity;
  size_t length;
} Vector;

enum TokenTag {
  TEOF,
  TINT,
  TIDENT,
  TPLUS,
  TMINUS,
  TASTERISK,
  TLPAREN,
  TRPAREN,
  TSEMICOLON,
};

typedef struct {
  enum TokenTag tag;
  int integer;
  char* ident;
} Token;

enum NodeTag {
    NINT,
    NPLUS,
    NMINUS,
    NMUL,
    NRETURN,
    NSTMTS,
};

typedef struct Node Node;
typedef struct Node {
  enum NodeTag tag;

  // bin op
  Node* lhs;
  Node* rhs;

  int integer;

  Node* ret;

  Vector* stmts;
} Node;

// node.c
Node* new_node(enum NodeTag tag);
Node* new_binop_node(enum NodeTag tag, Node* lhs, Node* rhs);
Node* new_int_node(int integer);
Node* new_return_node(Node* ret);
Node* new_stmts_node(Vector* stmts);
void dump_node(Node* node, int level);

// utils.c
char* format(const char* fmt, ...);
void error(const char* fmt, ...)__attribute__((noreturn));
void eprintf(const char* fmt, ...);
int streq(char* s0, char* s1);
Vector* new_vec();
void* vec_get(Vector* v, size_t i);
void vec_set(Vector* v, size_t i, void* elem);
void vec_push(Vector* v, void* elem);
void vec_pushi(Vector* v, intptr_t elem);
void* vec_pop(Vector* v);
intptr_t vec_popi(Vector* v);
char* vec_to_string(Vector* v);

// emit.c
typedef enum {
  AX = 0,
  DI,
  SI,
  DX,
  CX,
  R8,
  R9,
  R10,
  R11,
} Reg;

void emit_enter(int size, int nest);
void emit_leave();
void emit_mov(Reg dst, Reg src);
void compile(Node* node);

// lex.c
Vector* lex(FILE* file);
void dump_token(Token tok);

// parse.c
Node* parse(Vector* tokens);
