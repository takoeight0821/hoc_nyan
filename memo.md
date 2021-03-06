# register allocation

```
$0(4, 0 -> 0) $1(4, 0 -> 1)
$2(8, 0 -> 4) $3(8, 1 -> 9) $4(4, 2 -> 3) $5(4, 4 -> 6) $6(4, 5 -> 6) $7(4, 6 -> 7) $8(4, 8 -> 10) $9(4, 9 -> 10) $10(4, 10 -> 11)
=== f(.Lentry0) ===
.Lentry0: {
 [0] $1(4, 0 -> 1) = load $0(4, 0 -> 0)
 [1] ret $1(4, 0 -> 1)
}
=== main(.Lentry1) ===
params:
.Lentry1: {
 [0] $2(8, 0 -> 4) = alloc 4
 [1] $3(8, 1 -> 9) = alloc 4
 [2] $4(4, 2 -> 3) = 10
 [3] store $2(8, 0 -> 4) <- $4(4, 2 -> 3)
 [4] $5(4, 4 -> 6) = load $2(8, 0 -> 4)
 [5] $6(4, 5 -> 6) = 2
 [6] $7(4, 6 -> 7) = $5(4, 4 -> 6) + $6(4, 5 -> 6)
 [7] store $3(8, 1 -> 9) <- $7(4, 6 -> 7)
 [8] $8(4, 8 -> 10) = 1
 [9] $9(4, 9 -> 10) = load $3(8, 1 -> 9)
 [10] $10(4, 10 -> 11) = $8(4, 8 -> 10) + $9(4, 9 -> 10)
 [11] ret $10(4, 10 -> 11)
}
```

```
regs = { r10, r11, rbx, r12, r13, r14, r15 }
```

```
$0 = r10
$1 = r11
$2 = r10
$3 = r11
$4 = rbx
$5 = rbx
$6 = r10
$7 = r11
$8 = rbx
$9 = r10 
$10 = r11 
```

# TODO

<!-- 1. ヘッダファイルの用意 -->
<!-- 1. シフト演算 -->
<!-- 1. 不要な値のpopをNBLOCKでまとめてできるように変更 -->
<!-- 1. for文の修正(for(;;)みたいなのを許す) -->
<!-- 1. 空のreturn -->
<!-- 1. グローバル変数の初期化 -->
<!--    * emit_const -->
<!-- 1. NDEFVARのexprにNASSIGNをもたせるように変更 -->
<!-- 1. NDEREFの書き換えが多重にならないように修正 -->
<!-- 1. NLISTを削除（NDEFVARのexprにNCOMMAをもたせる） -->
<!-- 1. グローバル変数の入れ子になっていないリスト初期化 -->
1. va_list, va_startなどの実装
1. continue
1. ローカル変数の入れ子になったリスト初期化

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
