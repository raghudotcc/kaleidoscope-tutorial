#pragma once
#include "ExprAST.h"
#include <string>

class VariableExprAST: public ExprAST {
  std::string Name;

public:
  VariableExprAST(const std::string& Name) : Name(Name) {}
};
