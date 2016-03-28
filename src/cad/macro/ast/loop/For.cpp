#include "cad/macro/ast/loop/For.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
namespace loop {
void For::print_internals(IndentStream& os) const {
  os << "Variable:\n";
  if(variable) {
    os.indent() << *variable;
    os.dedent();

    os << "Variable initialization:\n";
    if(variable_init) {
      os.indent() << *variable_init;
      os.dedent();
    }
  }

  While::print_internals(os);

  os << "Operation:\n";
  if(operation) {
    os.indent() << *operation;
    os.dedent();
  }
}

For::For() {
}
For::For(const For& other)
    : While(other)
    , variable(other.variable)
    , variable_init(other.variable_init)
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
    return variable == other.variable && variable_init == other.variable_init &&
           operation == other.operation;
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
