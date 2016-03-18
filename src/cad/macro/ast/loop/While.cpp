#include "cad/macro/ast/loop/While.h"

#include "cad/macro/ast/Scope.h"

namespace cad {
namespace macro {
namespace ast {
namespace loop {
void While::print_internals(IndentStream& os) const {
  os << "Condition:\n";
  os.indent() << condition;
  os.dedent();

  if(scope) {
    os << "Scope:\n";
    os.indent() << *scope;
    os.dedent();
  }
}

While::While() {
}
While::While(const While& other)
    : AST(other)
    , condition(other.condition)
    , scope((other.scope) ? std::make_unique<Scope>(*other.scope) : nullptr) {
}
While::While(While&& other) {
  swap(*this, other);
}
While::While(parser::Token token)
    : AST(std::move(token)) {
}
While::~While() {
}

While& While::operator=(While other) {
  swap(*this, other);
  return *this;
}

bool While::operator==(const While& other) const {
  if(this == &other) {
    return true;
  } else if(AST::operator==(other)) {
    if(condition == other.condition) {
      if(scope && other.scope) {
        return *scope == *other.scope;
      } else if(!scope && !other.scope) {
        return true;
      }
    }
  }
  return false;
}
bool While::operator!=(const While& other) const {
  return !(*this == other);
}
}
}
}
}
