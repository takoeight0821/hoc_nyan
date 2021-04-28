#include "hoc.h"

Node *new_node(enum NodeTag tag, Token *token) {
  Node *node = calloc(1, sizeof(Node));
  node->tag = tag;
  node->token = token;
  return node;
}

size_t size_of(Type *ty) { return ty->size; }

Type *new_type(enum TypeTag tag, size_t size) {
  Type *ty = calloc(1, sizeof(Type));
  ty->ty = tag;
  ty->size = size;
  ty->align = size;
  return ty;
}

Type *clone_type(Type *t) {
  Type *new = calloc(1, sizeof(Type));
  memcpy(new, t, sizeof(Type));
  return new;
}

Node *clone_node(Node *node) {
  Node *new = calloc(1, sizeof(Node));
  memcpy(new, node, sizeof(Node));
  return new;
}

Type *void_type(void) { return new_type(TY_VOID, 0); }

Type *char_type(void) { return new_type(TY_CHAR, 1); }

Type *int_type(void) { return new_type(TY_INT, 4); }

Type *long_type(void) { return new_type(TY_LONG, 8); }

Type *ptr_to(Type *type) {
  Type *new_ty = new_type(TY_PTR, 8);
  new_ty->ptr_to = type;
  return new_ty;
}

Type *array_of(Type *type, size_t size) {
  Type *array = ptr_to(type);
  array->array_size = size;
  array->size = size * size_of(type);
  return array;
}

Type *type_of(Node *node) { return node->type; }

Field *look_struct(Field *fields, char *name) {
  for (Field *f = fields; f != NULL; f = f->next) {
    if (streq(f->name, name)) {
      return f;
    }
  }
  for (Field *f = fields; f != NULL; f = f->next) {
    if (f->type->fields) {
      // Unnamed substructures
      Field *sub = look_struct(f->type->fields, name);
      if (sub) {
        Field *g = calloc(1, sizeof(Field));
        g->name = sub->name;
        g->type = sub->type;
        g->next = sub->next;
        g->offset = f->offset + sub->offset;
        return g;
      }
    }
  }
  return NULL;
}

Type *field_type(Field *fields, char *name) {
  Field *field = look_struct(fields, name);
  if (!field)
    error("unreachable(field_type)");
  return field->type;
}

size_t field_offset(Field *fields, char *name) {
  Field *field = look_struct(fields, name);
  if (!field)
    error("unreachable(field_offset)");
  return field->offset;
}
