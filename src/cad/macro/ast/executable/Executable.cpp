#include "cad/macro/ast/executable/Executable.h"

#include "cad/macro/ast/Variable.h"

namespace cad {
namespace macro {
namespace ast {
namespace executable {
Executable::Executable(parser::Token token)
    : AST(std::move(token)) {
}

void Executable::print_internals(IndentStream& os) const {
  os << "parameter:\n";
  if(!parameter.empty()) {
    os.indent();
    for(const auto& v : parameter) {
      v.match([&os](const Variable& p) { os << p; },
              [&os](const Executable& p) { os << p; });
    }
    os.dedent();
    os << "\n";
  }
}

bool Executable::operator==(const Executable& other) const {
  if(this == &other) {
    return true;
  } else if(AST::operator==(other)) {
    return parameter == other.parameter;
  }
  return false;
}

bool Executable::operator!=(const Executable& other) const {
  return !(*this == other);
}
}
}
}
}
