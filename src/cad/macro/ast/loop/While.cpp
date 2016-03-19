#include "cad/macro/ast/loop/While.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
namespace loop {
void While::print_internals(IndentStream& os) const {
  Condition::print_internals(os);

  if(scope) {
    os << "Scope:\n";
    os.indent() << *scope;
    os.dedent();
  }
}

While::While() {
}
While::While(const While& other)
    : Condition(other)
    , scope((other.scope) ? std::make_unique<Scope>(*other.scope) : nullptr) {
}
While::While(While&& other) {
  swap(*this, other);
}
While::While(parser::Token token)
    : Condition(std::move(token)) {
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
  } else if(Condition::operator==(other)) {
    if(scope && other.scope) {
      return *scope == *other.scope;
    } else if(!scope && !other.scope) {
      return true;
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
