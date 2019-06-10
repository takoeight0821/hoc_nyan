#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef struct {
  void **ptr;
  size_t capacity;
  size_t length;
} Vector;

typedef struct {
  Vector *keys;
  Vector *vals;
} Map;

enum TokenTag {
  TEOF,
  TINT,
  TIDENT,
  TPLUS,
  TMINUS,
  TASTERISK,
  TSLASH,
  TEQUAL,
  TLPAREN,
  TRPAREN,
  TLBRACE,
  TRBRACE,
  TSEMICOLON,
  TCOMMA,
  TEQ,
  TNE,
  TLE,
  TGE,
  TLT,
  TGT,
};

typedef struct {
  enum TokenTag tag;
  int integer;
  char* ident;
} Token;

enum NodeTag {
    NINT,
    NVAR,
    NPLUS,
    NMINUS,
    NMUL,
    NDIV,
    NLT,
    NLE,
    NGT,
    NGE,
    NEQ,
    NNE,
    NDEFVAR,
    NASSIGN,
    NCALL,
    NRETURN,
    NIF,
    NIFELSE,
    NWHILE,
    NFOR,
    NBLOCK,
    NFUNCDEF,
};

typedef struct Node Node;
typedef struct Node {
  enum NodeTag tag;

  // binop and assign
  Node* lhs;
  Node* rhs;

  // integer literal
  int integer;

  // variable, function call, variable definition, function definition
  char* name;
  size_t offset; // variable

  // function call
  Vector* args;

  // return
  Node* ret;

  // block
  Vector* stmts;

  // if else
  Node* cond; // while, for
  Node* then;
  Node* els;

  // for
  Node* init;
  Node* step;

  // function definition
  Vector* params;
  Node* body; // while, for
  size_t local_size;
} Node;

// node.c
Node* new_node(enum NodeTag tag);
void dump_node(Node* node, int level);

// utils.c
char* format(const char* fmt, ...);
void error(const char* fmt, ...)__attribute__((noreturn));
void eprintf(const char* fmt, ...);
int streq(char* s0, char* s1);

// containers.c
Vector* new_vec();
void vec_push(Vector* v, void* elem);
void vec_pushi(Vector* v, intptr_t elem);
void* vec_pop(Vector* v);
intptr_t vec_popi(Vector* v);
char* vec_to_string(Vector* v);
Map* new_map();
void map_put(Map *map, char *key, void *val);
void map_puti(Map *map, char *key, intptr_t val);
void *map_get(Map *map, char *key);
intptr_t map_geti(Map *map, char *key);
int map_has_key(Map *map, char *key);

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
Vector* parse(Vector* tokens);
