# 9cc
- [低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)

# Diary
## 2020/5/31 昼
- 放置していたCコンパイラ作成を再開し始める。
- Step 4まで教科書通りに進めていたが、LR法で実装したくなったので[GitHub](https://github.com/rui314/course2020/tree/master)を見つつ、進めることにした。
- [エラー実装](https://github.com/rui314/course2020/blob/ac3a9f7c455d292eeef4a6ded0e5fa119ff49f0c/main.c)
- va_list:可変個の実引数を扱うための情報を保持するための型
- va_start:可変長引数の初期化
- vfprintf:ストリーム, 書式文字列, 変数を格納するための可変長引数
- strncmp:C言語の文字列比較
- isspace:標準空白文字かのチェック
- [LR法](https://ja.wikipedia.org/wiki/LR%E6%B3%95)
- [再帰下降構文解析](https://ja.wikipedia.org/wiki/%E5%86%8D%E5%B8%B0%E4%B8%8B%E9%99%8D%E6%A7%8B%E6%96%87%E8%A7%A3%E6%9E%90)
- [EmacsでC/C++用にclangdとclang-formatを使う - Qiita](https://qiita.com/kari_tech/items/4754fac39504dccfd7be)

## 2020/5/31 夜
- Level 5:四則演算実装
- ispunct
- isdigit
- プロトタイプ宣言をするとエラーが防げる
- レジスタやスタックしている認識を持ちながら、四則演算を実装する。

## 2020/6/1 深夜
- movzbってなんじゃらほい。
> ALというのは本書のここまでに登場していない新しいレジスタ名ですが、実はALはRAXの下位8ビットを指す別名レジスタにすぎません。従ってseteがALに値をセットすると、自動的にRAXも更新されることになります。ただし、RAXをAL経由で更新するときに上位56ビットは元の値のままになるので、RAX全体を0か1にセットしたい場合、上位56ビットはゼロクリアする必要があります。それを行うのがmovzb命令です。sete命令が直接RAXに書き込めればよいのですが、seteは8ビットレジスタしか引数に取れない仕様になっているので、比較命令では、このように2つの命令を使ってRAXに値をセットすることになります。

- pushってなんじゃらほい。
> PUSH命令は引数の要素をスタックトップに積むものとします。ここでは使用しませんが、スタックトップから要素を1つ取り除いて捨てるPOPという命令も考えることができます。

- gccのやつだとスタックポインタとベースポインタが用意され、初期化され4ごとに配置されていた

## 2020/6/1 夜
- https://github.com/rui314/course2020/commits/master?after=9f17954a6429daccd2992b04acaaf983d9e2ecd7+174
```
Cのstaticキーワードは、主に次の2つの用途で使われます。
1.ローカル変数にstaticをつけて、関数を抜けた後でも値が保存されるようにする
2.グローバル変数や関数にstaticをつけて、その変数や関数のスコープをファイルスコープにする
```
