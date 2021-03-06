#ifdef __hoc__
typedef long size_t;
typedef long ptrdiff_t;
typedef char bool;
void* malloc(size_t size);
void* calloc(size_t count, size_t size);
void* realloc(void* old, size_t size);
void free(void* ptr);
#define NULL 0
#define false 0
#define true 1

#define assert(e) e

extern void* stderr;

int printf();
int fprintf();
int exit();
int putchar(char c);
int puts(char* msg);

typedef void FILE;
void* memcpy(void* dst, void* src, size_t size);
#define EOF -1

char* strchr(char* s, int c);
int fgetc(FILE* stream);
int isdigit(int c);
int isalnum(int c);
int isalpha(int c);
int strcmp(char* s1, char* s2);
size_t strlen(char* s);
FILE* fopen(char* path, char* mode);
int fclose(FILE* stream);

struct __va_list_elem {
  int gp_offset;
  int fp_offset;
  void* overflow_arg_area;
  void* reg_save_area;
};

typedef struct __va_list_elem va_list[1];
static void* __va_arg(struct __va_list_elem* ap) {
  char* reg_save_area = ap->reg_save_area;
  void* r = reg_save_area + ap->gp_offset;
  ap->gp_offset += 8;
  return r;
}

#define va_start(ap_, start) __hoc_builtin_va_start(ap_, start)
#define va_arg(ap_, type) *(type *)__va_arg(ap_)
#define va_end(ap_) (0)

int vprintf(char* format, va_list ap);
int vfprintf(FILE* stream, char* format, va_list ap);
int vsprintf(char* str, char* format, va_list ap);
int vsnprintf(char* str, size_t size, char* format, va_list ap);

char* dirname(char* path);
char* basename(char* path);

int getopt(int argc, char** argv, char* optstring);
extern int optind;

#endif
#ifndef __hoc__
#include <stdnoreturn.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <libgen.h>
#include <getopt.h>
#include <assert.h>
#endif

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
  TDIRECTIVE, // # directive
};

typedef struct Token {
  struct Token* next;
  enum TokenTag tag;
  int integer;
  char* ident;
  char* str; // string literal

  /* for error reporting */
  char* source;
  char* start;

  bool bol; // beginning of line
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
    NLOGNOT,
    NNOT,
    NAND,
    NOR,
    NXOR,
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
    NSWITCH,
    NCASE,
    NDEFAULT,
    NBREAK,
    NCAST,
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
  size_t align;
  size_t size;

  // Pointer
  struct Type* ptr_to;

  // Array
  size_t array_size;

  // Struct
  char* tag;
  Field* fields;
} Type;

struct Field {
  Field* next;
  char* name;
  Type* type;
  size_t offset;
};

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
  // (type *) expr
  struct Node* expr;

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

typedef struct Function {
  char* name;
  Type* ret_type;
  Node* body;
  Vector* params;
  size_t local_size;
  bool is_static;
  bool has_va_arg;
} Function;

typedef struct GVar {
  struct GVar *next;
  char* name;
  Type* type;
  Node* init;
  Vector* inits;
  bool is_extern;
} GVar;

typedef struct Program {
  Vector* funcs;
  GVar* globals;
} Program;

typedef struct IProgram {
  Vector* ifuncs;
  GVar* globals;
} IProgram;

typedef struct IReg {
  int real_reg;
  int id;
  size_t size;

  // for register allocation
  int def;
  int last_use;
  bool spill;
} IReg;

enum IRTag {
  IIMM,
  ILABEL,
  IADD,
  ISUB,
  IMUL,
  IDIV,
  IMOD,
  ILT,
  ILE,
  IGT,
  IGE,
  IEQ,
  INE,
  IAND,
  IOR,
  IXOR,
  INOT,
  IADDRESS,
  IALLOC,
  ILOAD,
  ISTOREARG,
  ISTORE,
  IMOV,
  ICALL,
  IBR,
  IJMP,
  IRET,
};

typedef struct Block {
  char* label;
  Vector* instrs;
} Block;

typedef struct IR {
  /* r0 = IIMM imm_int
     r0 = ILABEL label
     r0 = IADD r1 r2
     r0 = ISUB r1 r2
     r0 = IMUL r1 r2
     r0 = IDIV r1 r2
     r0 = IMOD r1 r2
     r0 = ILT r1 r2
     r0 = ILE r1 r2
     r0 = IGT r1 r2
     r0 = IGE r1 r2
     r0 = IEQ r1 r2
     r0 = INE r1 r2
     r0 = IAND r1 r2
     r0 = IOR r1 r2
     r0 = IXOR r1 r2
     r0 = INOT r1
     r0 = IADDRESS imm_int
     IALLOC imm_int
     r0 = ILOAD r1
     r0 = ISTOREARG imm_int size
     ISTORE r1 r2
     r0 = IMOV r1
     r0 = ICALL func_name args
     IBR r1 then els
     IJMP jump_to
     RET r1
   */
  enum IRTag op;
  int imm_int;
  size_t size;
  IReg* r0;
  IReg* r1;
  IReg* r2;
  char* label;
  char* jump_to;
  char* then;
  char* els;
  char* func_name;
  Vector* args;
} IR;

typedef struct IFunc {
  char* name;
  bool is_static;
  bool has_va_arg;
  Vector* params;
  Vector* blocks;
  char* entry_label;
} IFunc;

// ir.c
int count_stack_size(IFunc* func);
char* show_ireg(IReg* reg);
char* show_ir(IR* ir);
char* show_block(int* inst_count, Block* block);
char* show_ifunc(IFunc* ifunc);

// node.c
Node* new_node(enum NodeTag tag, Token* token);
Node* clone_node(Node* node);
Type* new_type(enum TypeTag tag, size_t size);
Type* clone_type(Type* t);

Type* void_type(void);
Type* char_type(void);
Type* int_type(void);
Type* long_type(void);
Type* ptr_to(Type* type);
Type* array_of(Type* type, size_t size);

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
  AX,
  DI,
  SI,
  DX,
  CX,
  R8,
  R9,
  R10,
  R11,
} Reg;

void emit_x86(Program* prog);

// token.c
void warn_token(Token* tok, char* msg);
#ifdef __hoc__
void bad_token(Token* tok, char* msg);
#endif
#ifndef __hoc__
noreturn void bad_token(Token* tok, char* msg);
#endif
Token* lex(char* path);

// parse.c
Program* parse(Token* tokens);

// sema.c
void walk(Node* node);
void sema(Program* prog);

// utils.c
#ifdef __hoc__
void error(char* fmt, ...);
char *format(char* fmt, ...);
void eprintf(char* fmt, ...);
#endif
#ifndef __hoc__
noreturn void error(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
char *format(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
void eprintf(const char* fmt, ...);
#endif
int roundup(int x, int round_to);
int streq(char* s0, char* s1);

typedef struct StringBuilder {
  char* buf;
  size_t capacity;
  size_t length;
} StringBuilder;

StringBuilder* new_sb(void);
void sb_putc(StringBuilder* sb, char c);
void sb_puts(StringBuilder* sb, char* str);
char* sb_run(StringBuilder* sb);
void sb_destory(StringBuilder* sb);

bool eq_reserved(Token* token, char* name);

void dump_token(Token* tok);
void dump_node(Node* node, int level);
char* show_type(Type* ty);
void dump_type(Type* ty);
void dump_function(Function* func);

// cpp.c
Token* preprocess(char* dir, Token* tokens);

// ir.c
char* show_iprog(IProgram* iprog);
IReg* real_reg(Reg reg);
IR* new_ir(enum IRTag op);
IR* new_binop_ir(enum IRTag op, IReg* r0, IReg* r1, IReg* r2);

// gen_ir.c
IProgram* gen_ir(Program* program);

// regalloc.c
void alloc_regs(IProgram* prog);

// gen_x86.c
#define NUM_REGS 7
void gen_x86(IProgram* prog);
