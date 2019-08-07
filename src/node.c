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

Node* clone_node(Node* node) {
  Node* new = calloc(1, sizeof(Node));
  *new = *node;
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

Type* array_of(Type* type, size_t size) {
  Type* array = ptr_to(type);
  array->array_size = size;
  return array;
}

Type* type_of(Node* node) {
  return node->type;
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
