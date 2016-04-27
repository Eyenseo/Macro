#include "cad/macro/ast/callable/Return.h"

#include "cad/macro/ast/ValueProducer.h"

namespace cad {
namespace macro {
namespace ast {
namespace callable {
Return::Return() {
}
Return::Return(const Return& other)
    : AST(other)
    , output((other.output) ? std::make_unique<ValueProducer>(*other.output)
                            : nullptr) {
}
Return::Return(Return&& other) {
  swap(*this, other);
}
Return::Return(parser::Token token)
    : AST(std::move(token)) {
}
Return::~Return() {
}
Return& Return::operator=(Return other) {
  swap(*this, other);
  return *this;
}

void Return::print_internals(IndentStream& os) const {
  if(output) {
    eggs::match(output->value, [&os](const callable::Callable& o) { os << o; },
                [&os](const Variable& o) { os << o; },
                [&os](const Operator& o) { os << o; },
                [&os](const Literal<Literals::BOOL>& c) { os << c; },
                [&os](const Literal<Literals::INT>& c) { os << c; },
                [&os](const Literal<Literals::DOUBLE>& c) { os << c; },
                [&os](const Literal<Literals::STRING>& c) { os << c; });
  }
}

bool Return::operator==(const Return& other) const {
  if(this == &other) {
    return true;
  } else if(AST::operator==(other)) {
    if(output && other.output) {
      return *output == *other.output;
    } else if(!output && !other.output) {
      return true;
    }
  }
  return false;
}
bool Return::operator!=(const Return& other) const {
  return !(*this == other);
}
}
}
}
}
