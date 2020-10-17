#include "9cc.h"

// レジスタのtop
static int top;
// 引数のレジスタ 6変数まで
static char *argreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

// 数えてくれる
static int count(void) {
  static int i = 1;
  return i++;
}

// register操作
static char *reg(int idx) {
  static char *r[] = {"%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: %d", idx);
  return r[idx];
}

static void gen_expr(Node *node);

// lea dst, [src] : [src]のアドレス計算を行うが、メモリアクセスは行わずアドレス計算の結果そのものをdstにストア
// Pushes the given node's address to the stack.
static void gen_addr(Node *node) {
  switch (node->kind) {
  case ND_VAR:
    printf("  lea -%d(%%rbp), %s\n", node->var->offset, reg(top++));
    return;
  case ND_DEREF:
    gen_expr(node->lhs);
    return;
  }

  error("not an lvalue");
}

static void load(void) {
  printf("  mov (%s), %s\n", reg(top - 1), reg(top - 1));
}

static void store(void) {
  printf("  mov %s, (%s)\n", reg(top - 2), reg(top - 1));
  top--;
}

// Nodeから実行コードを出力する
// Generate code for a given node.
static void gen_expr(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  mov $%lu, %s\n", node->val, reg(top++));
    return;
  case ND_VAR:
    gen_addr(node);
    load();
    return;
  case ND_DEREF:
    gen_expr(node->lhs);
    load();
    return;
  case ND_ADDR:
    gen_addr(node->lhs);
    return;
  case ND_ASSIGN:
    gen_expr(node->rhs);
    gen_addr(node->lhs);
    store();
    return;
  case ND_FUNCALL: {
    int nargs = 0;
    for (Node *arg = node->args; arg; arg = arg->next) {
      gen_expr(arg);
      nargs++;
    }
    // 引数
    for (int i = 1; i <= nargs; i++)
      printf("  mov %s, %s\n", reg(--top), argreg[nargs - i]);
    printf("  push %%r10\n");
    printf("  push %%r11\n");
    printf("  mov $0, %%rax\n");
    printf("  call %s\n", node->funcname);
    printf("  pop %%r11\n");
    printf("  pop %%r10\n");
    printf("  mov %%rax, %s\n", reg(top++));
    return;
  }
  }

  gen_expr(node->lhs);
  gen_expr(node->rhs);

  char *rd = reg(top - 2);
  char *rs = reg(top - 1);
  top--;

  switch (node->kind) {
  case ND_ADD:
    printf("  add %s, %s\n", rs, rd);
    return;
  case ND_SUB:
    printf("  sub %s, %s\n", rs, rd);
    return;
  case ND_MUL:
    printf("  imul %s, %s\n", rs, rd);
    return;
  case ND_DIV:
    printf("  mov %s, %%rax\n", rd);
    printf("  cqo\n");
    printf("  idiv %s\n", rs);
    printf("  mov %%rax, %s\n", rd);
    return;
  case ND_EQ:
    printf("  cmp %s, %s\n", rs, rd);
    // フラグレジスタは通常の整数レジスタではないので、RAXに比較結果をセットしたい場合、フラグレジスタの特定のビットをRAXにコピーしてくる必要があります。
    //それを行うのがsete命令です。sete命令は、直前のcmp命令で調べた2つのレジスタの値が同じだった場合に、指定されたレジスタ（ここではAL）に1をセットします。それ以外の場合は0をセットします。

    // ALというのは本書のここまでに登場していない新しいレジスタ名ですが、実はALはRAXの下位8ビットを指す別名レジスタにすぎません。従ってseteがALに値をセットすると、自動的にRAXも更新されることになります。
    printf("  sete %%al\n");
    // ただし、RAXをAL経由で更新するときに上位56ビットは元の値のままになるので、RAX全体を0か1にセットしたい場合、上位56ビットはゼロクリアする必要があります。それを行うのがmovzb命令です。sete命令が直接RAXに書き込めればよいのですが、seteは8ビットレジスタしか引数に取れない仕様になっているので、比較命令では、このように2つの命令を使ってRAXに値をセットすることになります。
    printf("  movzx %%al, %s\n", rd);
    return;
  case ND_NE:
    printf("  cmp %s, %s\n", rs, rd);
    printf("  setne %%al\n");
    printf("  movzx %%al, %s\n", rd);
    return;
  case ND_LT:
    printf("  cmp %s, %s\n", rs, rd);
    printf("  setl %%al\n");
    printf("  movzx %%al, %s\n", rd);
    return;
  case ND_LE:
    printf("  cmp %s, %s\n", rs, rd);
    printf("  setle %%al\n");
    printf("  movzx %%al, %s\n", rd);
    return;
  default:
    error("invalid expression");
  }
}

static void gen_stmt(Node *node) {
  switch (node->kind) {
  case ND_IF: {
    int c = count();
    gen_expr(node->cond);
    printf("  cmp $0, %s\n", reg(--top));
    printf("  je  .L.else.%d\n", c);
    gen_stmt(node->then);
    printf("  jmp .L.end.%d\n", c);
    printf(".L.else.%d:\n", c);
    if (node->els)
      gen_stmt(node->els);
    printf(".L.end.%d:\n", c);
    return;
  }
  case ND_FOR: {
    int c = count();
    if (node->init)
      gen_stmt(node->init);
    printf(".L.begin.%d:\n", c);
    if (node->cond) {
      gen_expr(node->cond);
      printf("  cmp $0, %s\n", reg(--top));
      printf("  je  .L.end.%d\n", c);
    }
    gen_stmt(node->then);
    if (node->inc) {
      gen_expr(node->inc);
      top--;
    }
    printf("  jmp .L.begin.%d\n", c);
    printf(".L.end.%d:\n", c);
    return;
  }
  case ND_BLOCK:
    for (Node *n = node->body; n; n = n->next)
      gen_stmt(n);
    return;
  case ND_RETURN:
    gen_expr(node->lhs);
    printf("  mov %s, %%rax\n", reg(--top));
    printf("  jmp .L.return\n");
    return;
  case ND_EXPR_STMT:
    gen_expr(node->lhs);
    top--;
    return;
  default:
    error("invalid statement");
  }
}

void codegen(Function *prog) {
  printf(".globl main\n");
  printf("main:\n");

  // Prologue. %r12-15 are callee-saved registers.
  printf("  push %%rbp\n");
  printf("  mov %%rsp, %%rbp\n");
  printf("  sub $%d, %%rsp\n", prog->stack_size);
  printf("  mov %%r12, -8(%%rbp)\n");
  printf("  mov %%r13, -16(%%rbp)\n");
  printf("  mov %%r14, -24(%%rbp)\n");
  printf("  mov %%r15, -32(%%rbp)\n");

  gen_stmt(prog->body);
  assert(top == 0);

  // Epilogue
  printf(".L.return:\n");
  printf("  mov -8(%%rbp), %%r12\n");
  printf("  mov -16(%%rbp), %%r13\n");
  printf("  mov -24(%%rbp), %%r14\n");
  printf("  mov -32(%%rbp), %%r15\n");
  printf("  mov %%rbp, %%rsp\n");
  printf("  pop %%rbp\n");
  printf("  ret\n");
}
