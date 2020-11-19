#pragma once
#include "ExprAST.h"
#include "NumberExprAST.h"
#include "VariableExprAST.h"
#include "CallExprAST.h"
#include "BinaryExprAST.h"
#include "PrototypeAST.h"
#include "FunctionAST.h"
#include "Lexer/Lexer.h" 
#include <memory>
#include <string>
#include <utility>
#include <map>


// CurTok/getNextToken - Provide a simple token buffer. CurTok is the current
// token the parser is looking at. getNextToken reads another token from the 
// lexer and update CurTok with its results.
static int CurTok;
static int getNextToken() {
  return CurTok = gettok();
}

std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "LogError: %s\n", Str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}

// Basic Expression Parsing
static std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(NumVal);
  getNextToken();
  return std::move(Result);
}


static std::unique_ptr<ExprAST> ParseExpression();

// Parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken();
  auto V = ParseExpression();
  if (!V)
    return nullptr;

  // This might be happening here because we are doing a 
  // getNextToken in ParseExpression function
  if (CurTok != ')')
    return LogError("expected ')'");

  getNextToken(); // eat ')'; Remember that when we say eat something 
  // what we are saying is that get the next token after the something
  // that is to be eaten

  return V;
}


// identifierexpr
// ::= identifier
// ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
  std::string IdName = IdentifierStr;
  getNextToken();

  // This means that if the current token
  // is not '(' then it is not a function name
  // but a simple variable name
  if (CurTok != '(')
    return std::make_unique<VariableExprAST>(IdName);

  getNextToken(); // eat '('
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (CurTok != ')') {
    while (1) {
      if (auto Arg = ParseExpression()) 
        Args.push_back(std::move(Arg));
      else
        return nullptr;

      if (CurTok == ')')
        break;

      if (CurTok != ',')
        return LogError("Expected ')' or ',' in argument list");
      
      getNextToken();
    }
  }

  getNextToken();
  return std::make_unique<CallExprAST>(IdName, std::move(Args));
}


// primary
// ::= identifierexpr
// ::= numberexpr
// ::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
  switch(CurTok) {
  default:
      return LogError("unknown token when expecting an expression");
  case tok_identifier:
      return ParseIdentifierExpr();
  case tok_number:
      return ParseNumberExpr();
  case '(':
      return ParseParenExpr();
  }
}


// Binary Expression Parsing 
// Binary Expressions are significantly harder to parse because they are often ambiguous
// For eg. when given the string "x + y *z", the parse can choose to parse it as
// either "(x + y) * z" or "x + (y*z)". But the later is the standard and correct
// according to conventional mathematics, cause mul has higher precedence than addition
// There are many ways to handle this, but an elegant and efficient way is to use
// Operator Precendence Parsing. This parsing technique uses the precendence of binary 
// operators to guide recursion. To start with we need a table of precedence

/// BinopPrecedence - This holds the precendence for each binary operator that is
// defined

/// GetTokPrecendence - gett the precendence of the prending binary operator token.
static std::map<char, int> BinopPrecedence;
static int GetTokPrecedence() {
  if (!isascii(CurTok))
    return -1;

  int TokPrec = BinopPrecedence[CurTok];
  if (TokPrec <= 0) return -1;
  return TokPrec;
}

/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS) {
  // If this is a binop, find its precedence.
  while (true) {
    int TokPrec = GetTokPrecedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (TokPrec < ExprPrec)
      return LHS;

    // Okay, we know this is a binop.
    int BinOp = CurTok;
    getNextToken(); // eat binop

    // Parse the primary expression after the binary operator.
    auto RHS = ParsePrimary();
    if (!RHS)
      return nullptr;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }

    // Merge LHS/RHS.
    LHS =
        std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
  }
}


// expr
// ::= primary binorphs
//
static std::unique_ptr<ExprAST> ParseExpression() {
  auto LHS = ParsePrimary();
  if (!LHS)
    return nullptr;
  return ParseBinOpRHS(0, std::move(LHS));
}

// prototype
// ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
  if (CurTok != tok_identifier)
    return LogErrorP("Expected function name in prototype");

  std::string FnName = IdentifierStr;
  getNextToken();

  if (CurTok != '(')
    return LogErrorP("Expected '(' in prototype");

  std::vector<std::string> ArgNames;
  while(getNextToken() == tok_identifier)
    ArgNames.push_back(IdentifierStr);
  if (CurTok != ')')
    return LogErrorP("Expected ')' in prototype");

  getNextToken();

  return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

// definition
//  ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {
  getNextToken();
  auto Proto = ParsePrototype();
  if (!Proto) return nullptr;

  if (auto E = ParseExpression()) 
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
}

// external
//  ::= 'external' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {
  getNextToken();
  return ParsePrototype();
}

/// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    // Make an anonymous proto.
    auto Proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}


static void HandleDefinition() {
  if (ParseDefinition()) {
    fprintf(stderr, "Parsed a function definition.\n");
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

static void HandleExtern() {
  if (ParseExtern()) {
    fprintf(stderr, "Parsed an extern\n");
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

static void HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  if (ParseTopLevelExpr()) {
    fprintf(stderr, "Parsed a top-level expr\n");
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

// top
// ::= defintion | external | expression | ';'
static void MainLoop() {
	while (1) {
		fprintf(stderr, "ready> ");
		switch(CurTok) {
			case tok_eof:
				return;
			case ';':
				getNextToken();
				break;
		 	case tok_def:
				HandleDefinition();
				break;
			case tok_extern:
				HandleExtern();
				break;
			default:
				HandleTopLevelExpression();
				break;
		}
	}	
}
