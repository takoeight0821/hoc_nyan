# 配列とポインタ

```c
typedef struct Type {
  enum TypeTag ty;
  struct Type* ptr_to;
  size_t array_size;
} Type;
```

配列型`int [n]`は
`{ .ty = TY_PTR, .ptr_to = &{ TY_INT }, .array_size = n}`
で表す

`sizeof`演算子は`.array_size`を読むが、他の演算は単に`TY_PTR`として扱う
