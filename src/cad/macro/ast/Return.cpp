#include "cad/macro/ast/Return.h"

namespace cad {
namespace macro {
namespace ast {
Return::Return(parser::Token token)
    : AST(std::move(token)) {
}

void Return::print_internals(IndentStream& os) const {
  if(output) {
    // TODO check if this works https://llvm.org/bugs/show_bug.cgi?id=26929
    // TODO fixed in clang 3.8+
    // output->match([&os](const Executable& v) { os << v; },
    //               [&os](const Variable& v) { os << v; });

    switch(output->which()) {
    case 0:
      os << core::get<0>(*output);
      break;
    case 1:
      os << core::get<1>(*output);
      break;
    }
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
bool operator==(const Return& first, const AST& ast) {
  auto second = dynamic_cast<const Return*>(&ast);
  if(second) {
    return first == *second;
  }
  return false;
}
}
}
}
