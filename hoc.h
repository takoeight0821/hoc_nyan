#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct {
  void **ptr;
  size_t capacity;
  size_t length;
} Vector;

// vec.c
Vector* new_vec();
void* vec_get(Vector* v, size_t i);
void vec_set(Vector* v, size_t i, void* elem);
void vec_push(Vector* v, void* elem);
void* vec_pop(Vector* v);

enum TokenTag {
  TEOF,
  TINT,
  TPLUS,
};

typedef struct {
  enum TokenTag tag;
  int integer;
} Token;

enum NodeTag {
    NINT,
    NPLUS,
    NERROR,
};

typedef struct Node Node;
typedef struct Node {
  enum NodeTag tag;

  // bin op
  Node* lhs;
  Node* rhs;

  int integer;
} Node;

// node.c
Node* new_node(enum NodeTag tag);
Node* new_plus_node(Node* lhs, Node* rhs);
Node* new_int_node(int integer);
void dump_node(Node* node, int level);

// utils.c
char* format(const char* fmt, ...);
void error(const char* fmt, ...)__attribute__((noreturn));

// emit.c
void emit_mov(char* dst, char* src);
void emit_add(char* dst, char* src);
void compile(Node* node);

// lex.c
Vector* lex(FILE* file);
void dump_token(Token tok);

// parse.c
Node* parse(Vector* tokens);
