#include "9cc.h"

// 入力文字列
static char *current_input;

//
// Error Processings
//

// Reports an error and exit.
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit.
// Input:現在の入力の場所, 書式文字列, 変数を格納するための可変長引数
void verror_at(char *loc, char *fmt, va_list ap) {
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
void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}

// Consumes the current token if it matches `s`.
bool equal(Token *tok, char *s) {
  return strlen(s) == tok->len &&
         !strncmp(tok->loc, s, tok->len);
}

// Ensure that the current token is `s`.
Token *skip(Token *tok, char *s) {
  if (!equal(tok, s))
    error_tok(tok, "現在のtokenで'%s'が入力で期待されるのですが異なる模様です", s);
  return tok->next;
}

// Ensure that the current token is TK_NUM.
long get_number(Token *tok) {
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


// 2つのcharを比較し、一致すればtrue
static bool startswith(char *p, char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

// Alphabet判定
static bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// 数字判定
static bool is_alnum(char c) {
  return is_alpha(c) || ('0' <= c && c <= '9');
}

// キーワード判定
static bool is_keyword(Token *tok) {
  static char *kw[] = {"return", "if", "else", "for", "while"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    if (equal(tok, kw[i]))
      return true;
  return false;
}

static void convert_keywords(Token *tok) {
  for (Token *t = tok; t->kind != TK_EOF; t = t->next)
    if (t->kind == TK_IDENT && is_keyword(t))
      t->kind = TK_RESERVED;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
  Token head = {};
  Token *cur = &head;
  current_input = p;

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


    // Identifier
    if (is_alpha(*p)) {
      char *q = p++;
      while (is_alnum(*p))
        p++;
      cur = new_token(TK_IDENT, cur, q, p - q);
      continue;
    }

    // Multi-letter punctuators
    if (startswith(p, "==") || startswith(p, "!=") ||
        startswith(p, "<=") || startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // Single-letter punctuators
    if (ispunct(*p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    error_at(p, "構文解析中に不正なtokenを検出しました");
  }

  new_token(TK_EOF, cur, p, 0);
  convert_keywords(head.next);
  return head.next;
}
