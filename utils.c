#include "hoc.h"

char* format(const char *fmt, ...) {
  char* buf = calloc(2048, sizeof(char));
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, 2048, fmt, ap);
  va_end(ap);
  return realloc(buf, sizeof(char) * (strlen(buf) + 1));
}

void error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  exit(1);
}

void eprintf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

int streq(char* s0, char* s1) {
  return strcmp(s0, s1) == 0;
}

StringBuilder* new_sb(void) {
  StringBuilder* sb = calloc(1, sizeof(StringBuilder));
  sb->buf = calloc(8, sizeof(char));
  sb->capacity = 8;
  sb->length = 0;
  return sb;
}

void sb_putc(StringBuilder* sb, char c) {
  while (sb->length >= sb->capacity) {
    sb->capacity += 8;
    sb->buf = realloc(sb->buf, sizeof(char) * sb->capacity);
  }
  sb->buf[sb->length] = c;
  sb->length++;
}

void sb_puts(StringBuilder* sb, char* str) {
  for (size_t i = 0; i < strlen(str); i++) {
    sb_putc(sb, str[i]);
  }
}

char* sb_run(StringBuilder* sb) {
  sb_putc(sb, '\0');
  char* ret = sb->buf;
  sb_destory(sb);
  return ret;
}

void sb_destory(StringBuilder* sb) {
  free(sb);
}

void dump_token(Token* tok) {
  switch (tok->tag) {
  case TINT: {
    eprintf("%d", tok->integer);
    break;
  }
  case TSTRING: {
    eprintf("\"%s\"", tok->str);
    break;
  }
  case TIDENT: {
    eprintf("%s", tok->ident);
    break;
  }
  case TRESERVED: {
    eprintf("%s", tok->ident);
    break;
  }
  }
  if (tok->next != NULL) {
    eprintf(" ");
  }
}

static void show_type_(StringBuilder* sb, Type* ty) {
  switch (ty->ty) {
  case TY_VOID:
    sb_puts(sb, "void");
    break;
  case TY_CHAR:
    sb_puts(sb, "char");
    break;
  case TY_INT:
    sb_puts(sb, "int");
    break;
  case TY_LONG:
    sb_puts(sb, "long");
    break;
  case TY_PTR:
    sb_puts(sb, "ptr(");
    show_type_(sb, ty->ptr_to);
    sb_puts(sb, ")");
    break;
  case TY_STRUCT: {
    sb_puts(sb, format("struct %s {", ty->tag));
    for (Field* f = ty->fields; f != NULL; f = f->next) {
      sb_puts(sb, format("%s(+%zu) : ", f->name, f->offset));
      show_type_(sb, f->type);
      sb_puts(sb, ",");
    }
    sb_puts(sb, "}");
    break;
  }
  }
}

char* show_type(Type* ty) {
  StringBuilder* sb = new_sb();
  show_type_(sb, ty);
  return sb_run(sb);
}

void dump_type(Type* ty) {
  eprintf(show_type(ty));
}

static char* show_indent(int level) {
  return format("%*s", level, "");
}

static void indent(int level) {
  eprintf("%s", show_indent(level));
}

void dump_node(Node* node, int level) {
  switch (node->tag) {
  case NINT:
    indent(level);
    eprintf("%d\n", node->integer);
    break;
  case NVAR:
    indent(level);
    eprintf("%s : ", node->name);
    dump_type(node->type);
    eprintf("\n");
    break;
  case NGVAR:
    indent(level);
    eprintf("%s : ", node->name);
    dump_type(node->type);
    eprintf("\n");
    break;
  case NADD:
    indent(level);
    eprintf("(+\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NSUB:
    indent(level);
    eprintf("(-\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NMUL:
    indent(level);
    eprintf("(*\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NDIV:
    indent(level);
    eprintf("(/\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NMOD:
    indent(level);
    eprintf("(%%\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NLT:
    indent(level);
    eprintf("(<\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NLE:
    indent(level);
    eprintf("(<=\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NGT:
    indent(level);
    eprintf("(>\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NGE:
    indent(level);
    eprintf("(>=\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NEQ:
    indent(level);
    eprintf("(==\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NNE:
    indent(level);
    eprintf("(!=\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NNOT:
    indent(level);
    eprintf("(!\n");
    dump_node(node->expr, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NLOGAND:
    indent(level);
    eprintf("(&&\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NLOGOR:
    indent(level);
    eprintf("(||\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NDEFVAR: {
    indent(level);
    eprintf("(");
    dump_type(node->type);
    eprintf(" %s)\n", node->name);
    break;
  }
  case NASSIGN:
    indent(level);
    eprintf("(=\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NCALL: {
    indent(level);
    eprintf("(call\n");
    indent(level + 1);
    eprintf("%s\n", node->name);
    for (size_t i = 0; i < node->args->length; i++) {
      dump_node(node->args->ptr[i], level+1);
    }
    indent(level);
    eprintf(")\n");
    break;
  }
  case NADDR:
    indent(level);
    eprintf("(&\n");
    dump_node(node->expr, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NDEREF:
    indent(level);
    eprintf("(*\n");
    dump_node(node->expr, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NMEMBER:
    indent(level);
    eprintf("(.\n");
    dump_node(node->expr, level+1);
    indent(level+1);
    eprintf("%s\n", node->name);
    indent(level);
    eprintf(")\n");
    break;
  case NEXPR_STMT:
    indent(level);
    eprintf("(expr_stmt\n");
    dump_node(node->expr, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NRETURN:
    indent(level);
    eprintf("(return\n");
    dump_node(node->expr, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NIF:
    indent(level);
    eprintf("(if\n");
    dump_node(node->cond, level+1);
    dump_node(node->then, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NIFELSE:
    indent(level);
    eprintf("(if\n");
    dump_node(node->cond, level+1);
    dump_node(node->then, level+1);
    dump_node(node->els, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NWHILE:
    indent(level);
    eprintf("(while\n");
    dump_node(node->cond, level+1);
    dump_node(node->body, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NFOR:
    indent(level);
    eprintf("(for\n");
    dump_node(node->init, level+1);
    dump_node(node->cond, level+1);
    dump_node(node->step, level+1);
    dump_node(node->body, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NBLOCK: {
    indent(level);
    eprintf("{\n");
    for (size_t i = 0; i < node->stmts->length; i++) {
      dump_node(node->stmts->ptr[i], level+1);
    }
    indent(level);
    eprintf("}\n");
    break;
  }
  case NSIZEOF: {
    indent(level);
    eprintf("(sizeof\n");
    dump_node(node->expr, level+1);
    indent(level);
    eprintf(")\n");
    break;
  }
  case NSTRING: {
    indent(level);
    eprintf("\"%s\"\n", node->token->str);
    break;
  }
  case NSWITCH: {
    indent(level);
    eprintf("(switch\n");
    dump_node(node->expr, level + 1);
    dump_node(node->body, level+1);
    indent(level);
    eprintf(")\n");
    break;
  }
  case NCASE: {
    indent(level);
    eprintf("(case(%s)\n", node->name);
    dump_node(node->expr, level + 1);
    dump_node(node->body, level + 1);
    indent(level);
    eprintf(")\n");
    break;
  }
  case NDEFAULT: {
    indent(level);
    eprintf("(default(%s)\n", node->name);
    dump_node(node->body, level + 1);
    indent(level);
    eprintf(")\n");
  }
  case NBREAK: {
    indent(level);
    eprintf("(break)\n");
    break;
  }
  }
}

void dump_function(Function* func) {
  eprintf("%s ", func->name);
  eprintf("(");
  for (size_t i = 0; i < func->params->length; i++) {
    if (i != 0)
      eprintf(", ");
    dump_type(((Node*)func->params->ptr[i])->type);
    eprintf(" %s", ((Node*)func->params->ptr[i])->name);
  }
  eprintf(")\n");

  if (func->body)
    dump_node(func->body, 1);
}
