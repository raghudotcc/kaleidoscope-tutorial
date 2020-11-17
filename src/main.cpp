#include "lexer.h"
#include <iostream>

int main(int argc, char** argv) {
  auto val = gettok();
  std::cout << val << std::endl;
  return 0;
}
