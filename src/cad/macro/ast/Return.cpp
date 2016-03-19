#include "cad/macro/ast/Return.h"

namespace cad {
namespace macro {
namespace ast {
Return::Return(parser::Token token)
    : AST(std::move(token)) {
}

void Return::print_internals(IndentStream& os) const {
  if(output) {
    output->match([&os](const executable::Executable& o) { os << o; },
                  [&os](const Variable& o) { os << o; },
                  [&os](const UnaryOperator& o) { os << o; },
                  [&os](const BinaryOperator& o) { os << o; });
  }
}

bool Return::operator==(const Return& other) const {
  if(this == &other) {
    return true;
  } else if(AST::operator==(other)) {
    return output == other.output;
  }
  return false;
}
bool Return::operator!=(const Return& other) const {
  return !(*this == other);
}
}
}
}
