#include "cad/macro/ast/loop/For.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
namespace loop {
void For::print_internals(IndentStream& os) const {
  os << "Variable:\n";
  os.indent() << variable;
  os.dedent();

  While::print_internals(os);

  os << "Operation:\n";
  os.indent();
  operation.match([&os](const UnaryOperator& op) { os << op; },
                  [&os](const BinaryOperator& op) { os << op; },
                  [&os](const callable::Callable& op) { os << op; });
  os.dedent();
}

For::For() {
}
For::For(const For& other)
    : While(other)
    , variable(other.variable)
    , operation(other.operation) {
}
For::For(For&& other) {
  swap(*this, other);
}
For::For(parser::Token token)
    : While(std::move(token)) {
}
For::~For() {
}

For& For::operator=(For other) {
  swap(*this, other);
  return *this;
}

bool For::operator==(const For& other) const {
  if(this == &other) {
    return true;
  } else if(While::operator==(other)) {
    return variable == other.variable && operation == other.operation;
  }
  return false;
}
bool For::operator!=(const For& other) const {
  return !(*this == other);
}
}
}
}
}
