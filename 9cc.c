#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TK_RESERVED,  // 記号
  TK_NUM,       // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
  TokenKind kind;  // トークンの型
  Token *next;     // 次の入力トークン
  long val;        // kindがTK_NUMの場合、その数値
  char *loc;       // トークン文字列 Token location
  int len;         // Token length
};

// 入力文字列
static char *current_input;

// Error Handlings

// Reports an error and exit.
static void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit.
// Input:現在の入力の場所, 書式文字列, 変数を格納するための可変長引数
static void verror_at(char *loc, char *fmt, va_list ap) {
  int pos = loc - current_input;
  fprintf(stderr, "%s\n", current_input);
  // posの数だけ空白を出力する
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 特定の入力箇所に対してエラーメッセージを出力する
// Input:現在の入力の場所, 書式文字列
static void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
}

// 特定のトークンに対してエラーメッセージを出力する
static void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}



// Consumes the current token if it matches `s`.
static bool equal(Token *tok, char *s) {
  return strlen(s) == tok->len &&
         !strncmp(tok->loc, s, tok->len);
}

// Ensure that the current token is `s`.
static Token *skip(Token *tok, char *s) {
  if (!equal(tok, s))
    error_tok(tok, "現在のtokenで'%s'が入力で期待されるのですが異なる模様です", s);
  return tok->next;
}

// Ensure that the current token is TK_NUM.
static long get_number(Token *tok) {
  if (tok->kind != TK_NUM)
    error_tok(tok, "数が期待されるtokenで数以外のtokenを検出");
  return tok->val;
}

// Create a new token and add it as the next token of `cur`.
static Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}


// 入力文字列pをトークナイズしてそれを返す
static Token *tokenize(void) {
  Token head = {};
  Token *cur = &head;
  char *p = current_input;

  while (*p) {
    // Skip whitespace characters.
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Numeric literal
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtoul(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    // Punctuator
    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    error_at(p, "構文解析中に不正なtokenを検出しました");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}


int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  // 引数をglobal変数へ
  current_input = argv[1];
  // トークナイズする
  Token *tok = tokenize();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 式の最初は数でなければならないので、それをチェックして
  // 最初のmov命令を出力
  // The first token must be a number
  printf("  mov rax, %ld\n", get_number(tok));
  tok = tok->next;


  // `+ <数>`あるいは`- <数>`というトークンの並びを消費しつつ
  // アセンブリを出力
  while (tok->kind != TK_EOF) {
    if (equal(tok, "+")) {
      printf("  add rax, %ld\n", get_number(tok->next));
      tok = tok->next->next;
      continue;
    }

    tok = skip(tok, "-");
    printf("  sub rax, %ld\n", get_number(tok));
    tok = tok->next;
  }

printf("  ret\n");
  return 0;
}
