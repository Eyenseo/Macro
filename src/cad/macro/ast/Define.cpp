#include "cad/macro/ast/Define.h"

#include "cad/macro/ast/Scope.h"

namespace cad {
namespace macro {
namespace ast {
Define::Define(parser::Token token)
    : AST(std::move(token)) {
}

void Define::print_internals(IndentStream& os) const {
  definition.match([&os](const Function& d) { os << d; },
                   [&os](const EntryFunction& d) { os << d; },
                   [&os](const Variable& d) { os << d; });
}

bool Define::operator==(const Define& other) const {
  if(this == &other) {
    return true;
  } else if(AST::operator==(other)) {
    return definition == other.definition;
  }
  return false;
}

bool Define::operator!=(const Define& other) const {
  return !(*this == other);
}
}
}
}
