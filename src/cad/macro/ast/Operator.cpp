#include "cad/macro/ast/Operator.h"

#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
Operator::Operator(parser::Token token)
    : AST(std::move(token))
    , operation(Operation::NONE) {
}
Operator::Operator(const Operator& other)
    : AST(other)
    , left_operand((other.left_operand)
                       ? std::make_unique<ValueProducer>(*other.left_operand)
                       : nullptr)
    , right_operand((other.right_operand)
                        ? std::make_unique<ValueProducer>(*other.right_operand)
                        : nullptr)
    , operation(other.operation) {
}
Operator::Operator(Operator&& other) {
  swap(*this, other);
}
Operator& Operator::operator=(Operator other) {
  swap(*this, other);
  return *this;
}
void Operator::print_internals(IndentStream& os) const {
  if(left_operand) {
    os << "Left operand:\n";
    os.indent() << *left_operand;
    os.dedent();
  }
  os << "Operation: " << operation_to_string() << "\n";
  if(right_operand) {
    os << "Right operand:\n";
    os.indent() << *right_operand;
    os.dedent();
  }
}

std::string Operator::operation_to_string() const {
  switch(operation) {
  case Operation::NONE:
    return "NONE";
  case Operation::DIVIDE:
    return "divide";
  case Operation::MULTIPLY:
    return "multiply";
  case Operation::MODULO:
    return "modulo";
  case Operation::ADD:
    return "add";
  case Operation::SUBTRACT:
    return "subtract";
  case Operation::SMALLER:
    return "smaller";
  case Operation::SMALLER_EQUAL:
    return "smaller equal";
  case Operation::GREATER:
    return "greater";
  case Operation::GREATER_EQUAL:
    return "greater equal";
  case Operation::EQUAL:
    return "equal";
  case Operation::NOT_EQUAL:
    return "not equal";
  case Operation::AND:
    return "and";
  case Operation::OR:
    return "or";
  case Operation::ASSIGNMENT:
    return "assignment";
  case Operation::NOT:
    return "not";
  case Operation::PRINT:
    return "print";
  case Operation::TYPEOF:
    return "typeof";
  case Operation::NEGATIVE:
    return "negative";
  }
  return "This 'can not' happen - you accessed uninitialized memory or "
         "something similar!";
}

bool Operator::operator==(const Operator& other) const {
  if(this == &other) {
    return true;
  } else if(AST::operator==(other) && operation == other.operation) {
    if((left_operand && other.left_operand) &&
       (right_operand && other.right_operand)) {
      return *left_operand == *other.left_operand &&
             *right_operand == *other.right_operand;
    } else if((!left_operand && !other.left_operand) &&
              (right_operand && other.right_operand)) {
      return *right_operand == *other.right_operand;
    } else if((left_operand && other.left_operand) &&
              (!right_operand && !other.right_operand)) {
      return *left_operand == *other.left_operand;
    } else if((!left_operand && !other.left_operand) &&
              (!right_operand && !other.right_operand)) {
      return true;
    }
  }
  return false;
}
bool Operator::operator!=(const Operator& other) const {
  return !(*this == other);
}
}
}
}
