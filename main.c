#include "9cc.h"

// よくわからない
static int align_to(int n, int align) {
  return (n + align - 1) & ~(align - 1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }
  Token *tok = tokenize(argv[1]);
  Function *prog = parse(tok);

  // Assign offsets to local variables.
  int offset = 32; // 32 for callee-saved registers
  for (Var *var = prog->locals; var; var = var->next) {
    offset += 8;
    var->offset = offset;
  }

  // よくわからないけどヨシ！
  prog->stack_size = align_to(offset, 16);

  // Traverse the AST to emit assembly.
  codegen(prog);

  return 0;
}
