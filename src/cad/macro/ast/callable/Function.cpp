#include "cad/macro/ast/callable/Function.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/ValueProducer.h"
#include "cad/macro/ast/Variable.h"

namespace cad {
namespace macro {
namespace ast {
namespace callable {
Function::Function() {
}
Function::Function(const Function& other)
    : AST(other)
    , parameter(other.parameter)
    , scope((other.scope) ? std::make_unique<Scope>(*other.scope) : nullptr) {
}
Function::Function(Function&& other) {
  swap(*this, other);
}
Function::Function(parser::Token token)
    : AST(std::move(token)) {
}
Function::~Function() {
}
Function& Function::operator=(Function other) {
  swap(*this, other);
  return *this;
}

void Function::print_internals(IndentStream& os) const {
  os << "parameter:\n";
  if(!parameter.empty()) {
    os.indent();
    for(const auto& v : parameter) {
      os << v;
    }
    os.dedent();
  }
  if(scope) {
    os << *scope;
  }
}

bool Function::operator==(const Function& other) const {
  if(this == &other) {
    return true;
  } else if(AST::operator==(other)) {
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
}
}
}
}
