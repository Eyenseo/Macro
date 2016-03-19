#include "cad/macro/ast/executable/Executable.h"

#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
namespace executable {
Executable::Executable() {
}
Executable::Executable(const Executable& other)
    : AST(other)
    , parameter(other.parameter) {
}
Executable::Executable(Executable&& other) {
  swap(*this, other);
}
Executable::Executable(parser::Token token)
    : AST(std::move(token)) {
}
Executable::~Executable() {
}
Executable& Executable::operator=(Executable other) {
  swap(*this, other);
  return *this;
}

void Executable::print_internals(IndentStream& os) const {
  os << "parameter:\n";
  if(!parameter.empty()) {
    os.indent();
    for(const auto& v : parameter) {
      v.value.match([&os](const executable::Executable& o) { os << o; },
                    [&os](const Variable& o) { os << o; },
                    [&os](const UnaryOperator& o) { os << o; },
                    [&os](const BinaryOperator& o) { os << o; },
                    [&os](const Literal<Literals::BOOL>& c) { os << c; },
                    [&os](const Literal<Literals::INT>& c) { os << c; },
                    [&os](const Literal<Literals::DOUBLE>& c) { os << c; },
                    [&os](const Literal<Literals::STRING>& c) { os << c; });
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
