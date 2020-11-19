#include <iostream>
#include "Parser/Parser.h"

int main(int argc, char** argv) {
  // Install standard Binary operators
  // 1 is the lowest precedence
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 30;
  BinopPrecedence['*'] = 40;

  fprintf(stderr, "ready> ");
  getNextToken();

  MainLoop();

  return 0;
}
