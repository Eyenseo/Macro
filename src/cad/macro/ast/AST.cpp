#include "cad/macro/ast/AST.h"

#include "cad/macro/IndentStream.h"

namespace cad {
namespace macro {
namespace ast {
AST::AST() {
}
AST::AST(parser::Token t)
    : token(std::move(t)) {
}

bool AST::operator==(const AST& other) const {
  if(this == &other) {
    return true;
  } else {
    return token == other.token;
  }
}

bool AST::operator!=(const AST& other) const {
  return !(*this == other);
}
std::ostream& AST::operator<<(std::ostream& os) {
  print_token(os, "AST");
  return os;
}

void AST::print_token(std::ostream& os, std::string prefix) const {
  print_token(os, std::move(prefix), [](IndentStream&) {});
}
}
}
}
