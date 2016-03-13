#include "cad/macro/ast/executable/Function.h"

#include "cad/macro/ast/Scope.h"

namespace cad {
namespace macro {
namespace ast {
namespace executable {
Function::Function(parser::Token token)
    : AST(std::move(token)) {
}

void Function::set_scope(Scope scope) {
  scope_ = std::make_shared<Scope>(std::move(scope));
}

const Scope& Function::get_scope() const {
  return *scope_;
}
void Function::print_scope(IndentStream& os) const {
  if(scope_) {
    os << *scope_;
  }
}
}
}
}
}
