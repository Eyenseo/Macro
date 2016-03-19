#ifndef cad_macro_ast_Operator_h
#define cad_macro_ast_Operator_h

#include "cad/macro/ast/AST.h"

#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/executable/Executable.h"

#include <core/variant.hpp>
#include <core/optional.hpp>

namespace cad {
namespace macro {
namespace ast {
enum class OperationType { Unary, Binary };
enum class UnaryOperation { NONE, NOT };
enum class BinaryOperation {
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
  ASSIGNMENT
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
  UnaryOperator(UnaryOperator&& other);
  UnaryOperator(parser::Token token);

  UnaryOperator& operator=(UnaryOperator other);

  friend void swap(UnaryOperator& first, UnaryOperator& second) {
    // enable ADL (not necessary in our case, but good practice)
    using std::swap;

    swap(static_cast<Operator&>(first), static_cast<Operator&>(second));
    swap(first.operand, second.operand);
  }


  std::string operation_to_string() const;

  bool operator==(const UnaryOperator& other) const;
  bool operator!=(const UnaryOperator& other) const;

  friend std::ostream& operator<<(std::ostream& os, const UnaryOperator& ast) {
    ast.print_token(os, "UnaryOperator",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};

class BinaryOperator : public Operator<OperationType::Binary> {
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<Operand> left_operand;
  std::unique_ptr<Operand> right_operand;

public:
  BinaryOperator() = default;
  BinaryOperator(const BinaryOperator& other);
  BinaryOperator(BinaryOperator&& other);
  BinaryOperator(parser::Token token);

  BinaryOperator& operator=(BinaryOperator other);
  friend void swap(BinaryOperator& first, BinaryOperator& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<Operator&>(first), static_cast<Operator&>(second));
    swap(first.left_operand, second.left_operand);
    swap(first.right_operand, second.right_operand);
  }

  std::string operation_to_string() const;

  bool operator==(const BinaryOperator& other) const;
  bool operator!=(const BinaryOperator& other) const;

  friend std::ostream& operator<<(std::ostream& os, const BinaryOperator& ast) {
    ast.print_token(os, "BinaryOperator",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};

using ValueVariant = core::variant<executable::Executable, Variable,
                                   UnaryOperator, BinaryOperator>;

struct Operand {
  ValueVariant value;

public:
  Operand() = default;
  Operand(ValueVariant op)
      : value(std::move(op)) {
  }
  Operand(executable::Executable op)
      : value(std::move(op)) {
  }
  Operand(Variable op)
      : value(std::move(op)) {
  }
  Operand(UnaryOperator op)
      : value(std::move(op)) {
  }
  Operand(BinaryOperator op)
      : value(std::move(op)) {
  }

  bool operator==(const Operand& other) const;
  bool operator!=(const Operand& other) const;

  friend std::ostream& operator<<(std::ostream& os, const Operand& op) {
    op.value.match([&os](const executable::Executable& o) { os << o; },
                   [&os](const Variable& o) { os << o; },
                   [&os](const UnaryOperator& o) { os << o; },
                   [&os](const BinaryOperator& o) { os << o; });
    return os;
  }
};
}
}
}
#endif
