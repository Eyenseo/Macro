#include "cad/macro/ast/loop/For.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
namespace loop {
void For::print_internals(IndentStream& os) const {
  os << "Define:\n";
  if(define) {
    os.indent() << *define;
    os.dedent();
  }
  os << "Variable initialization:\n";
  if(variable) {
    os.indent() << *variable;
    os.dedent();
  }

  While::print_internals(os);

  os << "Operation:\n";
  if(operation) {
    os.indent() << *operation;
    os.dedent();
  }
}

For::For(parser::Token token)
    : While(std::move(token)) {
}

bool For::operator==(const For& other) const {
  if(this == &other) {
    return true;
  } else if(While::operator==(other)) {
    return define == other.define && variable == other.variable &&
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
