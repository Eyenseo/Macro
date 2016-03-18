#include "cad/macro/ast/logic/Condition.h"

namespace cad {
namespace macro {
namespace ast {
namespace logic {
Condition::Condition(parser::Token token)
    : AST(std::move(token)) {
}

void Condition::print_internals(IndentStream& os) const {
  os << "Contition:\n";
  if(condition) {
    os.indent();
    condition->match([&os](const UnaryOperator& c) { os << c; },
                     [&os](const BinaryOperator& c) { os << c; },
                     [&os](const Variable& c) { os << c; },
                     [&os](const executable::Executable& c) { os << c; });
    os.dedent();
  }
}

bool Condition::operator==(const Condition& other) const {
  if(this == &other) {
    return true;
  } else if(AST::operator==(other)) {
    if(condition && other.condition) {
      return condition == other.condition;
    } else if(!condition && !other.condition) {
      return true;
    }
  }
  return false;
}
bool Condition::operator!=(const Condition& other) const {
  return !(*this == other);
}
}
}
}
}
