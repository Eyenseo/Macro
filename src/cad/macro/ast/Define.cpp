#include "cad/macro/ast/Define.h"

#include "cad/macro/ast/Scope.h"

namespace cad {
namespace macro {
namespace ast {
Define::Define(parser::Token token)
    : AST(std::move(token)) {
}

void Define::print_internals(IndentStream& os) const {
  if(definition) {
    // TODO check if this works https://llvm.org/bugs/show_bug.cgi?id=26929
    // TODO fixed in clang 3.8+
    // definition->match([&os](const Function& v) { os << v; },
    //                   [&os](const EntryFunction& v) { os << v; });

    switch(definition->which()) {
    case 0:
      os << core::get<0>(*definition);
      break;
    case 1:
      os << core::get<1>(*definition);
      break;
    case 2:
      os << core::get<2>(*definition);
      break;
    }
  }
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

bool operator==(const Define& first, const AST& ast) {
  auto second = dynamic_cast<const Define*>(&ast);
  if(second) {
    return first == *second;
  }
  return false;
}
}
}
}
