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
- [LR法](https://ja.wikipedia.org/wiki/LR%E6%B3%95)
- [再帰下降構文解析](https://ja.wikipedia.org/wiki/%E5%86%8D%E5%B8%B0%E4%B8%8B%E9%99%8D%E6%A7%8B%E6%96%87%E8%A7%A3%E6%9E%90)
- []「EmacsでC/C++用にclangdとclang-formatを使う - Qiita」
https://qiita.com/kari_tech/items/4754fac39504dccfd7be[]「EmacsでC/C++用にclangdとclang-formatを使う - Qiita」
https://qiita.com/kari_tech/items/4754fac39504dccfd7be
