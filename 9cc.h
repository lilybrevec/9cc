#include <assert.h>
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

// プロトタイプ宣言
bool equal(Token*, char*);
Token *skip(Token*, char*);
Token *tokenize(char *input);
long get_number(Token*);

void error(char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind; // Node kind
  Node *lhs;     // Left-hand side
  Node *rhs;     // Right-hand side
  long val;      // Used if kind == ND_NUM
};

Node *parse(Token *tok);

//
// codegen.c
//

void codegen(Node *node);