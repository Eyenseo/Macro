#include "cad/macro/ast/logic/Condition.h"

#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
namespace logic {
Condition::Condition() {
}
Condition::Condition(const Condition& other)
    : AST(other)
    , condition((other.condition)
                    ? std::make_unique<ValueProducer>(*other.condition)
                    : nullptr) {
}
Condition::Condition(Condition&& other) {
  swap(*this, other);
}
Condition::Condition(parser::Token token)
    : AST(std::move(token)) {
}
Condition::~Condition() {
}
Condition& Condition::operator=(Condition other) {
  swap(*this, other);
  return *this;
}

void Condition::print_internals(IndentStream& os) const {
  os << "Contition:\n";
  if(condition) {
    os.indent();
    condition->value.match(
        [&os](const callable::Callable& o) { os << o; },
        [&os](const Variable& o) { os << o; },
        [&os](const Operator& o) { os << o; },
        [&os](const Literal<Literals::BOOL>& c) { os << c; },
        [&os](const Literal<Literals::INT>& c) { os << c; },
        [&os](const Literal<Literals::DOUBLE>& c) { os << c; },
        [&os](const Literal<Literals::STRING>& c) { os << c; });
    os.dedent();
  }
}

bool Condition::operator==(const Condition& other) const {
  if(this == &other) {
    return true;
  } else if(AST::operator==(other)) {
    if(condition && other.condition) {
      return *condition == *other.condition;
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
