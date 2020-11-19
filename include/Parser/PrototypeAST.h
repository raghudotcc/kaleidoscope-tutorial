#pragma once

#include <string>
#include <vector>

class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;
public:
  PrototypeAST(const std::string &name, std::vector<std::string> Args)
    : Name(name), Args(std::move(Args)) {}
  const std::string &getName() const { return Name; }
};
