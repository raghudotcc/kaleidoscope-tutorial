#pragma once
#include "ExprAST.h"
#include <string>
#include <vector>
#include <memory>

class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public: 
  CallExprAST(const std::string &Callee,
        std::vector<std::unique_ptr<ExprAST>> Args) 
    : Callee(Callee), Args(std::move(Args)) {}
};
