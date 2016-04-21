#ifndef cad_macro_ast_Operator_h
#define cad_macro_ast_Operator_h

#include "cad/macro/ast/AST.h"

#include <memory>

namespace cad {
namespace macro {
namespace ast {
enum class Operation {
  NONE,
  DIVIDE,
  MULTIPLY,
  MODULO,
  ADD,
  SUBTRACT,
  SMALLER,
  SMALLER_EQUAL,
  GREATER,
  GREATER_EQUAL,
  EQUAL,
  NOT_EQUAL,
  AND,
  OR,
  ASSIGNMENT,
  NOT,
  PRINT,
  TYPEOF,
  NEGATIVE
};
class ValueProducer;

class Operator : public AST {
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<ValueProducer> left_operand;
  std::unique_ptr<ValueProducer> right_operand;
  Operation operation;

public:
  Operator() = default;
  Operator(const Operator& other);
  Operator(Operator&& other);
  Operator(parser::Token token);

  Operator& operator=(Operator other);
  friend void swap(Operator& first, Operator& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<AST&>(first), static_cast<AST&>(second));
    swap(first.left_operand, second.left_operand);
    swap(first.right_operand, second.right_operand);
    swap(first.operation, second.operation);
  }

  std::string operation_to_string() const;

  bool operator==(const Operator& other) const;
  bool operator!=(const Operator& other) const;

  friend std::ostream& operator<<(std::ostream& os, const Operator& ast) {
    if(ast.left_operand && ast.right_operand) {
      ast.print_token(os, "BinaryOperator",
                      [&ast](IndentStream& os) { ast.print_internals(os); });
    } else if(ast.left_operand || ast.right_operand) {
      ast.print_token(os, "UnaryOperator",
                      [&ast](IndentStream& os) { ast.print_internals(os); });
    } else {
      ast.print_token(os, "EmptyOperator",
                      [&ast](IndentStream& os) { ast.print_internals(os); });
    }
    return os;
  }
};
}
}
}
#endif
