#include "cad/macro/ast/callable/Callable.h"

#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
namespace callable {
Callable::Callable() {
}
Callable::Callable(const Callable& other)
    : AST(other)
    , parameter(other.parameter) {
}
Callable::Callable(Callable&& other) {
  swap(*this, other);
}
Callable::Callable(parser::Token token)
    : AST(std::move(token)) {
}
Callable& Callable::operator=(Callable other) {
  swap(*this, other);
  return *this;
}

void Callable::print_internals(IndentStream& os) const {
  os << "parameter:\n";
  if(!parameter.empty()) {
    os.indent();
    for(const auto& v : parameter) {
      os << v.first.token.token << ": ";
      eggs::match(v.second.value,
                  [&os](const callable::Callable& o) { os << o; },
                  [&os](const Variable& o) { os << o; },
                  [&os](const Operator& o) { os << o; },
                  [&os](const Literal<Literals::BOOL>& c) { os << c; },
                  [&os](const Literal<Literals::INT>& c) { os << c; },
                  [&os](const Literal<Literals::DOUBLE>& c) { os << c; },
                  [&os](const Literal<Literals::STRING>& c) { os << c; });
    }
    os.dedent();
  }
}

bool Callable::operator==(const Callable& other) const {
  if(this == &other) {
    return true;
  } else if(AST::operator==(other)) {
    return parameter == other.parameter;
  }
  return false;
}

bool Callable::operator!=(const Callable& other) const {
  return !(*this == other);
}
void swap(Callable& first, Callable& second) {
  // enable ADL
  using std::swap;

  swap(static_cast<AST&>(first), static_cast<AST&>(second));
  swap(first.parameter, second.parameter);
}
}
}
}
}
