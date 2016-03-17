#include "cad/macro/ast/executable/operator/Operator.h"

namespace cad {
namespace macro {
namespace ast {
namespace executable {
namespace operation {
//////////////////////////////////////////
// UnaryOperator
//////////////////////////////////////////
UnaryOperator::UnaryOperator(const UnaryOperator& other)
    : Operator<OperationType::Unary>(other)
    , operand((other.operand) ? std::make_unique<Operand>(*other.operand)
                              : nullptr) {
}
UnaryOperator::UnaryOperator(parser::Token token)
    : Operator<OperationType::Unary>(std::move(token)) {
}

void UnaryOperator::print_internals(IndentStream& os) const {
  os << "Operation: " << operation_to_string() << "\n";
  if(operand) {
    os.indent() << "Operand:\n";
    os << *operand;
    os.dedent();
    os << "\n";
  }
}

std::string UnaryOperator::operation_to_string() const {
  switch(operation) {
  case UnaryOperation::NONE:
    return "NONE";
  case UnaryOperation::NOT:
    return "not";
  }

  return "This 'can not' happen - you accessed uninitialized memory or "
         "something similar!";
}

bool UnaryOperator::operator==(const UnaryOperator& other) const {
  if(this == &other) {
    return true;
  } else if(Operator::operator==(other)) {
    if(operand && other.operand) {
      return *operand == *other.operand;
    } else if(!operand && !other.operand) {
      return true;
    }
  }
  return false;
}
bool UnaryOperator::operator!=(const UnaryOperator& other) const {
  return !(*this == other);
}

bool operator==(const UnaryOperator& first, const AST& ast) {
  auto second = dynamic_cast<const UnaryOperator*>(&ast);
  if(second) {
    return first == *second;
  }
  return false;
}

//////////////////////////////////////////
// BinaryOperator
//////////////////////////////////////////
BinaryOperator::BinaryOperator(const BinaryOperator& other)
    : Operator<OperationType::Binary>(other)
    , left_operand((other.left_operand)
                       ? std::make_unique<Operand>(*other.left_operand)
                       : nullptr)
    , right_operand((other.right_operand)
                        ? std::make_unique<Operand>(*other.right_operand)
                        : nullptr) {
}
BinaryOperator::BinaryOperator(parser::Token token)
    : Operator<OperationType::Binary>(std::move(token)) {
}

void BinaryOperator::print_internals(IndentStream& os) const {
  if(left_operand) {
    os << "Left operand:\n";
    os.indent() << *left_operand;
    os.dedent();
    os << "\n";
  }
  os << "Operation: " << operation_to_string() << "\n";
  if(right_operand) {
    os << "Right operand:\n";
    os.indent() << *right_operand;
    os.dedent();
    os << "\n";
  }
}

std::string BinaryOperator::operation_to_string() const {
  switch(operation) {
  case BinaryOperation::NONE:
    return "NONE";
  case BinaryOperation::AND:
    return "and";
  case BinaryOperation::OR:
    return "or";
  case BinaryOperation::EQUAL:
    return "equal";
  case BinaryOperation::GREATER:
    return "greater";
  case BinaryOperation::GREATER_EQUAL:
    return "greater equal";
  case BinaryOperation::SMALLER:
    return "smaller";
  case BinaryOperation::SMALLER_EQUAL:
    return "smaller equal";
  case BinaryOperation::DIVIDE:
    return "divide";
  case BinaryOperation::MULTIPLY:
    return "multiply";
  case BinaryOperation::ADD:
    return "add";
  case BinaryOperation::MODULO:
    return "modulo";
  }
  return "This 'can not' happen - you accessed uninitialized memory or "
         "something similar!";
}

bool BinaryOperator::operator==(const BinaryOperator& other) const {
  if(this == &other) {
    return true;
  } else if(Operator::operator==(other)) {
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
bool BinaryOperator::operator!=(const BinaryOperator& other) const {
  return !(*this == other);
}

bool operator==(const BinaryOperator& first, const AST& ast) {
  auto second = dynamic_cast<const BinaryOperator*>(&ast);
  if(second) {
    return first == *second;
  }
  return false;
}

//////////////////////////////////////////
// Operand
//////////////////////////////////////////
bool Operand::operator==(const Operand& other) const {
  if(this == &other) {
    return true;
  } else {
    return operand == other.operand;
  }
}
bool Operand::operator!=(const Operand& other) const {
  return !(*this == other);
}
}
}
}
}
}
