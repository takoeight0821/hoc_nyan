#include "hoc.h"

Node* new_node(enum NodeTag tag, Token* token) {
  Node* node = calloc(1, sizeof(Node));
  node->tag = tag;
  node->token = token;
  return node;
}

size_t size_of(Type* ty) {
  switch (ty->ty) {
  case TY_VOID:
    return 0;
  case TY_CHAR:
    return 1;
  case TY_INT:
    return 4;
  case TY_LONG:
    return 8;
  case TY_PTR:
    if (ty->array_size == 0) {
      return 8;
    } else {
      // ty->array_size >= 1のときは配列型
      return ty->array_size * size_of(ty->ptr_to);
    }
  case TY_STRUCT: {
    size_t s = 0;
    for (Field* f = ty->fields; f != NULL; f = f->next) {
      s += size_of(f->type);
    }
    return s;
  }
  }

  error("unreachable(size_of)");
}

Type* new_type(void) {
  Type* ty = calloc(1, sizeof(Type));
  return ty;
}

Type* clone_type(Type* t) {
  Type* new = new_type();
  *new = *t;
  return new;
}

Type* void_type(void) {
  Type* t = new_type();
  t->ty = TY_VOID;
  return t;
}

Type* char_type(void) {
  Type* t = new_type();
  t->ty = TY_CHAR;
  return t;
}

Type* int_type(void) {
  Type* t = new_type();
  t->ty = TY_INT;
  return t;
}

Type* long_type(void) {
  Type* t = new_type();
  t->ty = TY_LONG;
  return t;
}

Type* ptr_to(Type* type) {
  Type* new_ty = calloc(1, sizeof(Type));
  new_ty->ty = TY_PTR;
  new_ty->ptr_to = type;
  return new_ty;
}

Type* type_of(Node* node) {
  if (node->type) {
    return node->type;
  } else {
    error("node must be type checked");
  }
}

Type* field_type(Field* fields, char* name) {
  for (Field* f = fields; f != NULL; f = f->next) {
    if (streq(f->name, name)) {
      return f->type;
    }
  }
  error("unreachable(field_type)");
}

size_t field_offset(Field* fields, char* name) {
  for (Field* f = fields; f != NULL; f = f->next) {
    if (streq(f->name, name)) {
      return f->offset;
    }
  }
  error("unreachable(field_offset)");
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
  case NPLUS:
    indent(level);
    eprintf("(+\n");
    dump_node(node->lhs, level+1);
    dump_node(node->rhs, level+1);
    indent(level);
    eprintf(")\n");
    break;
  case NMINUS:
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
    eprintf("(case\n");
    dump_node(node->expr, level + 1);
    indent(level);
    eprintf(")\n");
    break;
  }
  case NBREAK: {
    indent(level);
    eprintf("(break)");
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
