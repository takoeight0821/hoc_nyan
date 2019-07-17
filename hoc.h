#include <assert.h>
#include <stdnoreturn.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

extern char* src;

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
  TAND,
  TEQUAL,
  TLPAREN,
  TRPAREN,
  TLBRACE,
  TRBRACE,
  TLBRACK,
  TRBRACK,
  TSEMICOLON,
  TCOMMA,
  TEQ,
  TNE,
  TLE,
  TGE,
  TLT,
  TGT,
  TSTRING,
  TDOT,
};

typedef struct {
  enum TokenTag tag;
  int integer;
  char* ident;
  char* str; // string literal
  char* start; // for error reporting
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
    NADDR,
    NDEREF,
    NEXPR_STMT,
    NRETURN,
    NIF,
    NIFELSE,
    NWHILE,
    NFOR,
    NBLOCK,
    NSIZEOF,
    NSTRING,
};

enum TypeTag {
  TY_INT,
  TY_CHAR,
  TY_PTR,
  TY_STRUCT,
};

typedef struct Type {
  enum TypeTag ty;

  // Pointer
  struct Type* ptr_to;

  // Array
  size_t array_size;

  // Struct
  char* tag;
  Map* struct_fields; // Map<Type*>
  size_t field_offset;
} Type;

typedef struct {
  Type* type;
  char* name;
  size_t offset;
  bool is_local;
} Var;

/* typedef struct { */
/*   Type* type; */
/*   char* name; */
/* } GVar; */

typedef struct Node {
  enum NodeTag tag;
  Token* token; // for error reporting

  Type* type; // type
  struct Node* lhs; // left-hand side
  struct Node* rhs; // right-hand side
  int integer; // integer literal
  Vector* stmts; // block
  struct Node* expr; // "return" or expression stmt or address-of or dereference
  size_t str_id; // string literal

  char* name; // function call, variable definition

  Var* var;
  /* GVar* gvar; */

  // function call
  Vector* args;

  // "if" ( cond ) then "else" els
  // "for" ( init; cond; step ) body
  // "while" ( cond ) body
  struct Node* cond;
  struct Node* then;
  struct Node* els;
  struct Node* init;
  struct Node* step;
  struct Node* body;

} Node;

typedef struct {
  char* name;
  Type* ret_type;
  Node* body;
  Vector* params;
  size_t local_size;
} Function;

typedef struct {
  Vector* funcs;
  Map* globals;
  Vector* strs;
} Program;

// node.c
Node* new_node(enum NodeTag tag, Token* token);
Type* new_type(void);
void dump_node(Node* node, int level);
void dump_type(Type* ty);
void dump_function(Function* func);
Type* type_of(Node* node);
size_t size_of(Type* ty);

// utils.c

noreturn void error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
char *format(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
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

void gen_x86(Program* prog);

// token.c
void warn_token(Token* tok, char* msg);
noreturn void bad_token(Token* tok, char* msg);
void dump_token(Token* tok);
Vector* lex(FILE* file);

// parse.c
Program* parse(Vector* tokens);

// sema.c
void sema(Program* prog);
