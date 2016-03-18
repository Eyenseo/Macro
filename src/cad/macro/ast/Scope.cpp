#include "cad/macro/ast/Scope.h"

#include "cad/macro/ast/Define.h"

#include "cad/macro/IndentStream.h"

namespace cad {
namespace macro {
namespace ast {
Scope::Scope(parser::Token token)
    : AST(std::move(token)) {
}

void Scope::print_internals(std::ostream& os) const {
  IndentStream indent_os(os);
  for(auto& v : nodes) {
    v.match([&os](const EntryFunction& n) { os << n; },   //
            [&os](const Executable& n) { os << n; },      //
            [&os](const Function& n) { os << n; },        //
            [&os](const Return& n) { os << n; },          //
            [&os](const Scope& n) { os << n; },           //
            [&os](const Define& n) { os << n; },          //
            [&os](const UnaryOperator& n) { os << n; },   //
            [&os](const BinaryOperator& n) { os << n; },  //
            [&os](const If& n) { os << n; },              //
            [&os](const While& n) { os << n; },           //
            [&os](const DoWhile& n) { os << n; },         //
            [&os](const For& n) { os << n; });            //
  }
}

bool Scope::operator==(const Scope& other) const {
  if(this == &other) {
    return true;
  } else if(AST::operator==(other)) {
    return nodes == other.nodes;
  }
  return false;
}

bool Scope::operator!=(const Scope& other) const {
  return !(*this == other);
}
}
}
}
