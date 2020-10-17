#define _POSIX_C_SOURCE 200809L
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
  TK_IDENT,     // 識別子
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

// Local variable
typedef struct Var Var;
struct Var {
  Var *next;
  char *name; // Variable name
  int offset; // Offset from RBP
};

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_ASSIGN,  // =
  ND_RETURN, // return
  ND_IF,        // "if"
  ND_FOR,       // "for" and while
  ND_BLOCK,     // { ... }
  ND_EXPR_STMT, // Expression statement
  ND_VAR, // Variable
  ND_NUM, // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind; // Node kind
  Node *next;    // Next node
  Node *lhs;     // Left-hand side
  Node *rhs;     // Right-hand side

  // "if" and "for" and "while" statement
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *inc;

  // Block
  Node *body;
  long val;      // Used if kind == ND_NUM
  Var *var;      // Used if kind == ND_VAR
};

typedef struct Function Function;
struct Function {
  Node *body;
  Var *locals;
  int stack_size;
};

Function *parse(Token *tok);

//
// codegen.c
//

void codegen(Function *prog);
