# TODO

<!-- 1. ヘッダファイルの用意 -->
<!-- 1. シフト演算 -->
<!-- 1. 不要な値のpopをNBLOCKでまとめてできるように変更 -->
1. for文の修正(for(;;)みたいなのを許す)
1. va_list, va_startなどの実装
1. グローバル変数の初期化
1. リスト初期化

# メモ

Restrictは無視しても良い
Switch-case エスケープ文字　複合演算子
プリプロセッサ
トークンにどのマクロで展開されたかのリストを持たせる
Unionはメンバのオフセットが全部0のstruct

Tokenのタグはキーワード、リテラル、ユーザー定義識別子の3つぐらいがよさそう。
`=`は`TEQUAL`、なら`==`は何にする？みたいなことを考えたくない。

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

# switch-case

switchノードにcaseノードへのポインタのVectorを持たせる。
emitかsemaで各caseノードにラベルを割り当て、.nameフィールドに書き込む。
