#include "cad/macro/ast/logic/If.h"

#include "cad/macro/ast/Scope.h"

namespace cad {
namespace macro {
namespace ast {
namespace logic {
void If::print_internals(IndentStream& os) const {
  Condition::print_internals(os);

  if(true_scope) {
    os << "True:\n";
    os.indent() << *true_scope;
    os.dedent();
  }
  if(false_scope) {
    os << "False:\n";
    os.indent() << *false_scope;
    os.dedent();
  }
}

If::If(parser::Token token)
    : Condition(std::move(token)) {
}
If::If(const If& other)
    : cad::macro::ast::logic::Condition(other)
    , true_scope((other.true_scope) ? std::make_unique<Scope>(*other.true_scope)
                                    : nullptr)
    , false_scope((other.false_scope)
                      ? std::make_unique<Scope>(*other.false_scope)
                      : nullptr) {
}
If::If(If&& other) {
  swap(*this, other);
}
If::~If() {
}
If& If::operator=(If other) {
  swap(*this, other);
  return *this;
}


bool If::operator==(const If& other) const {
  if(this == &other) {
    return true;
  } else if(Condition::operator==(other)) {
    if((true_scope && other.true_scope) && (false_scope && other.false_scope)) {
      return *true_scope == *other.true_scope &&
             *false_scope == *other.false_scope;
    } else if((!true_scope && !other.true_scope) &&
              (false_scope && other.false_scope)) {
      return *false_scope == *other.false_scope;
    } else if((true_scope && other.true_scope) &&
              (!false_scope && !other.false_scope)) {
      return *true_scope == *other.true_scope;
    } else if((!true_scope && !other.true_scope) &&
              (!false_scope && !other.false_scope)) {
      return true;
    }
  }
  return false;
}
bool If::operator!=(const If& other) const {
  return !(*this == other);
}
}
}
}
}
