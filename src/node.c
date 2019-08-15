#include <hoc.h>

Node* new_node(enum NodeTag tag, Token* token) {
  Node* node = calloc(1, sizeof(Node));
  node->tag = tag;
  node->token = token;
  return node;
}

size_t size_of(Type* ty) {
  return ty->size;
}

Type* new_type(enum TypeTag tag, size_t size) {
  Type* ty = calloc(1, sizeof(Type));
  ty->ty = tag;
  ty->size = size;
  ty->align = size;
  return ty;
}

Type* clone_type(Type* t) {
  Type* new = calloc(1, sizeof(Type));
  memcpy(new, t, sizeof(Type));
  return new;
}

Node* clone_node(Node* node) {
  Node* new = calloc(1, sizeof(Node));
  memcpy(new, node, sizeof(Node));
  return new;
}

Type* void_type(void) {
  return new_type(TY_VOID, 0);
}

Type* char_type(void) {
  return new_type(TY_CHAR, 1);
}

Type* int_type(void) {
  return new_type(TY_INT, 4);
}

Type* long_type(void) {
  return new_type(TY_LONG, 8);
}

Type* ptr_to(Type* type) {
  Type* new_ty = new_type(TY_PTR, 8);
  new_ty->ptr_to = type;
  return new_ty;
}

Type* array_of(Type* type, size_t size) {
  Type* array = ptr_to(type);
  array->array_size = size;
  array->size = size * size_of(type);
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
