# TODO

1. Mapは型安全性がつらいので、linked listで置き換えれるとこは置き換える
1. ヘッダファイルの用意
1. enum
1. シフト演算
1. switch-case

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

# RAXレジスタの使い方

今は単なる汎用レジスタ
スタックトップの値をRAXに保存するようにすれば簡単に性能が改善できるんじゃ？

# ローカル変数の初期化式

<!-- NDEFVARに`Node*`を持たせる。initフィールドを使う？ -->
パース時に`T a = x;`を`T a; a = x;`に変換する。
配列や構造体の初期化は`Vector*<Node*>`を持たせる。

# 構造体

構造体はどうしよう？
ローカル変数を確保するのと同じ感覚でスタックに確保すれば良い。
引数と返り値はとりあえずポインタ経由のみ扱う。

構造体名と定義のマップが必要。
パース時に作っとく。
