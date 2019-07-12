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

# グローバル変数

```c
enum NodeTag {
  NGVar
}

typedef struct {
    Type* type;
    char* name;
} GVar;

typedef struct {
    GVar* gvar;
} Node;
```

```c
static Map* global_env; // Map(char*, GVar)
```

グローバル変数の宣言が出てくると、GVarを生成してglobal_envに追加。
変数名がlocal_envに無かったらglobal_envからGVarをルックアップしてNGVarを作成。
あとはいい感じにコード生成する。
