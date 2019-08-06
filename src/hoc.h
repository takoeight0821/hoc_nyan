#include <stdnoreturn.h>
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

enum TokenTag {
  TINT,      // int literal
  TSTRING,   // string literal
  TIDENT,    // identifier
  TRESERVED, // keyword or punctuator
};

typedef struct Token {
  struct Token* next;
  enum TokenTag tag;
  int integer;
  char* ident;
  char* str; // string literal
  char* start; // for error reporting
} Token;

enum NodeTag {
    NINT,
    NVAR,
    NGVAR,
    NADD,
    NSUB,
    NMUL,
    NDIV,
    NMOD,
    NLT,
    NLE,
    NGT,
    NGE,
    NEQ,
    NNE,
    NNOT,
    NLOGAND,
    NLOGOR,
    NCOMMA,
    NDEFVAR,
    NASSIGN,
    NCALL,
    NADDR,
    NDEREF,
    NMEMBER,
    NEXPR_STMT,
    NRETURN,
    NIF,
    NIFELSE,
    NWHILE,
    NFOR,
    NBLOCK,
    NSIZEOF,
    NSTRING,
    NSWITCH,
    NCASE,
    NDEFAULT,
    NBREAK,
};

enum TypeTag {
  TY_VOID,
  TY_CHAR,
  TY_INT,
  TY_LONG,
  TY_PTR,
  TY_STRUCT,
};

typedef struct Field Field;

typedef struct Type {
  enum TypeTag ty;

  // Pointer
  struct Type* ptr_to;

  // Array
  size_t array_size;

  // Struct
  char* tag;
  Field* fields;
} Type;

typedef struct Field {
  Field* next;
  char* name;
  Type* type;
  size_t offset;
} Field;

typedef struct Node {
  enum NodeTag tag;
  Token* token; // for error reporting

  Type* type; // type
  struct Node* lhs; // left-hand side
  struct Node* rhs; // right-hand side
  int integer; // integer literal
  Vector* stmts; // block

  // "return" expr
  // expr ";"
  // "&" expr
  // "*" expr
  // expr "." name
  struct Node* expr;
  size_t str_id; // string literal

  char* name; // function call, variable definition, variable

  size_t offset; // local variable

  // function call
  Vector* args;

  // "if" ( cond ) then "else" els
  // "for" ( init; cond; step ) body
  // "while" ( cond ) body
  // "switch" ( expr ) body
  // "case" expr ":" body
  // "default" ":" body
  struct Node* cond;
  struct Node* then;
  struct Node* els;
  struct Node* init;
  struct Node* step;
  struct Node* body;

  Vector* cases; // for switch-case
} Node;

typedef struct {
  char* name;
  Type* ret_type;
  Node* body;
  Vector* params;
  size_t local_size;
  bool is_static;
} Function;

typedef struct GVar {
  struct GVar *next;
  char* name;
  Type* type;
  bool is_extern;
} GVar;

typedef struct {
  Vector* funcs;
  GVar* globals;
  Vector* strs;
} Program;

// node.c
Node* new_node(enum NodeTag tag, Token* token);
Node* clone_node(Node*);
Type* new_type(void);
Type* clone_type(Type*);

Type* void_type(void);
Type* char_type(void);
Type* int_type(void);
Type* long_type(void);
Type* ptr_to(Type* type);

Type* type_of(Node* node);
size_t size_of(Type* ty);
Type* field_type(Field* fields, char* name);
size_t field_offset(Field* fields, char* name);

// containers.c
Vector* new_vec();
void vec_push(Vector* v, void* elem);
void* vec_pop(Vector* v);
void* vec_last(Vector* v);

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
void bad_token(Token* tok, char* msg);
Token* lex(char* path);

// parse.c
Program* parse(Token* tokens);

// sema.c
void sema(Program* prog);

// utils.c
noreturn void error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
char *format(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void eprintf(const char* fmt, ...);
int streq(char* s0, char* s1);

typedef struct {
  char* buf;
  size_t capacity;
  size_t length;
} StringBuilder;

StringBuilder* new_sb(void);
void sb_putc(StringBuilder*, char);
void sb_puts(StringBuilder*, char*);
char* sb_run(StringBuilder*);
void sb_destory(StringBuilder*);

void dump_token(Token* tok);
void dump_node(Node* node, int level);
char* show_type(Type* ty);
void dump_type(Type* ty);
void dump_function(Function* func);