#include "cad/macro/ast/loop/DoWhile.h"

namespace cad {
namespace macro {
namespace ast {
namespace loop {
DoWhile::DoWhile(parser::Token token)
    : While(std::move(token)) {
}

bool DoWhile::operator==(const DoWhile& other) const {
  return While::operator==(other);
}
bool DoWhile::operator!=(const DoWhile& other) const {
  return !(*this == other);
}

bool operator==(const DoWhile& first, const AST& ast) {
  auto second = dynamic_cast<const DoWhile*>(&ast);
  if(second) {
    return first == *second;
  }
  return false;
}
}
}
}
}
