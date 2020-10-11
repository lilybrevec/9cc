#include "9cc.h"
// 文字列をトークン化するプログラム

// 入力文字列
static char *current_input;

// エラー処理
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Input:現在の入力の場所, 書式文字列, 変数を格納するための可変長引数
// エラー時の書式なども考慮して出力する。
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

// エラー処理を最初に実行する関数
// 特定の入力箇所に対してエラーメッセージを出力する
// Input:現在の入力の場所, 書式文字列
void error_at(char *loc, char *fmt, ...) {
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

// 文字列sに一致するトークンかの判定
// Consumes the current token if it matches `s`.
bool equal(Token *tok, char *s) {
  return strlen(s) == tok->len &&
         !strncmp(tok->loc, s, tok->len);
}

// Ensure that the current token is `s`.
// IFの次はELSEが来るのが確定なので、次に飛ばす
Token *skip(Token *tok, char *s) {
  if (!equal(tok, s))
    error_tok(tok, "現在のtokenで'%s'が入力で期待されるのですが異なる模様です", s);
  return tok->next;
}

// Ensure that the current token is TK_NUM.
// 次のトークンとして数が来ないといけない。返却値は数。
long get_number(Token *tok) {
  if (tok->kind != TK_NUM)
    error_tok(tok, "数が期待されるtokenで数以外のtokenを検出");
  return tok->val;
}

// 新しいトークンを作成する
static Token *new_token(TokenKind kind, char *start, char *end) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = start;
  tok->len = end - start;
  return tok;
}


// 2つのcharを比較し、一致すればtrue
static bool startswith(char *p, char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

// Returns true if c is valid as the first character of an identifier.
static bool is_ident1(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// Returns true if c is valid as a non-first character of an identifier.
static bool is_ident2(char c) {
  return is_ident1(c) || ('0' <= c && c <= '9');
}

// Read a punctuator token from p and returns its length.
static int read_punct(char *p) {
  if (startswith(p, "==") || startswith(p, "!=") ||
      startswith(p, "<=") || startswith(p, ">="))
    return 2;

  return ispunct(*p) ? 1 : 0;
}

static bool is_keyword(Token *tok) {
  static char *kw[] = {"return", "if", "else"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    if (equal(tok, kw[i]))
      return true;
  return false;
}

// キーワードを変換する
static void convert_keywords(Token *tok) {
  for (Token *t = tok; t->kind != TK_EOF; t = t->next)
    if (is_keyword(t))
      t->kind = TK_KEYWORD;
}


// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
  Token head = {};
  Token *cur = &head;
  current_input = p;

  while (*p) {
    // 空白文字をスキップする。
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Numeric literal
    if (isdigit(*p)) {
      cur = cur->next = new_token(TK_NUM, p, p);
      char *q = p;
      // strtoulはstringをunsigned long intに変換する。
      cur->val = strtoul(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    // Identifier or keyword
    if (is_ident1(*p)) {
      char *start = p;
      do {
        p++;
      } while (is_ident2(*p));
      cur = cur->next = new_token(TK_IDENT, start, p);
      continue;
    }

    // Punctuators
    int punct_len = read_punct(p);
    if (punct_len) {
      cur = cur->next = new_token(TK_PUNCT, p, p + punct_len);
      p += cur->len;
      continue;
    }

    error_at(p, "invalid token");
  }

  cur = cur->next = new_token(TK_EOF, p, p);
  convert_keywords(head.next);
  return head.next;
}
