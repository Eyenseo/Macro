#ifndef cad_macro_ast_logic_Operator_h
#define cad_macro_ast_logic_Operator_h

#include "cad/macro/ast/AST.h"

#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/executable/Executable.h"

#include <core/variant.hpp>
#include <core/optional.hpp>

namespace cad {
namespace macro {
namespace ast {
namespace executable {
namespace operation {
enum class OperationType { Unary, Binary };
enum class UnaryOperation { NONE, NOT };
enum class BinaryOperation {
  NONE,
  AND,
  OR,
  EQUAL,
  GREATER,
  GREATER_EQUAL,
  SMALLER,
  SMALLER_EQUAL,
  DIVIDE,
  MULTIPLY,
  ADD,
  MODULO
};
struct Operand;

template <OperationType T>
class Operator : public AST {
public:
  using Operation =
      typename std::conditional<T == OperationType::Unary, UnaryOperation,
                                BinaryOperation>::type;
  Operation operation;

public:
  Operator()
      : AST()
      , operation(Operation::NONE) {
  }
  Operator(parser::Token token)
      : AST(std::move(token))
      , operation(Operation::NONE) {
  }

  bool operator==(const Operator& other) const {
    if(this == &other) {
      return true;
    } else if(AST::operator==(other)) {
      return operation == other.operation;
    }
    return false;
  }
  bool operator!=(const Operator& other) const {
    return !(*this == other);
  }
};

class UnaryOperator : public Operator<OperationType::Unary> {
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<Operand> operand;

public:
  UnaryOperator() = default;
  UnaryOperator(const UnaryOperator& other);
  UnaryOperator(parser::Token token);

  std::string operation_to_string() const;

  bool operator==(const UnaryOperator& other) const;
  bool operator!=(const UnaryOperator& other) const;

  friend std::ostream& operator<<(std::ostream& os, const UnaryOperator& ast) {
    ast.print_token(os, "UnaryOperator",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
bool operator==(const UnaryOperator& first, const AST& second);

class BinaryOperator : public Operator<OperationType::Binary> {
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<Operand> left_operand;
  std::unique_ptr<Operand> right_operand;

public:
  BinaryOperator() = default;
  BinaryOperator(const BinaryOperator& other);
  BinaryOperator(parser::Token token);


  std::string operation_to_string() const;

  bool operator==(const BinaryOperator& other) const;
  bool operator!=(const BinaryOperator& other) const;

  friend std::ostream& operator<<(std::ostream& os, const BinaryOperator& ast) {
    ast.print_token(os, "BinaryOperator",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
bool operator==(const BinaryOperator& first, const AST& second);

struct Operand {
  using OperandMember =
      core::variant<Executable, Variable, UnaryOperator, BinaryOperator>;
  OperandMember operand;

  bool operator==(const Operand& other) const;
  bool operator!=(const Operand& other) const;

  friend std::ostream& operator<<(std::ostream& os, const Operand& operand) {
    operand.operand.match([&os](const Executable& o) { os << o; },
                          [&os](const Variable& o) { os << o; },
                          [&os](const UnaryOperator& o) { os << o; },
                          [&os](const BinaryOperator& o) { os << o; });
    return os;
  }
};
}
}
}
}
}
#endif
