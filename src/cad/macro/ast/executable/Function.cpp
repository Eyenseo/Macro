#include "cad/macro/ast/executable/Function.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/Variable.h"

namespace cad {
namespace macro {
namespace ast {
namespace executable {
Function::Function() {
}
Function::Function(parser::Token token)
    : Executable(std::move(token)) {
}
Function::Function(const Function& other)
    : Executable(other)
    , scope((other.scope) ? std::make_unique<Scope>(*other.scope) : nullptr) {
}
Function::~Function() {
}

void Function::print_internals(IndentStream& os) const {
  Executable::print_internals(os);
  if(scope) {
    os << *scope;
  }
}
bool Function::operator==(const Function& other) const {
  if(this == &other) {
    return true;
  } else if(Executable::operator==(other)) {
    if(scope && other.scope) {
      return *scope == *other.scope;
    } else if(!scope && !other.scope) {
      return true;
    }
  }
  return false;
}
bool Function::operator!=(const Function& other) const {
  return !(*this == other);
}
bool operator==(const Function& first, const AST& ast) {
  auto second = dynamic_cast<const Function*>(&ast);
  if(second) {
    return first == *second;
  }
  return false;
}
}
}
}
}
