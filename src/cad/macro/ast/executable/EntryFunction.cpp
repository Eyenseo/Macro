#include "cad/macro/ast/executable/EntryFunction.h"

#include "cad/macro/ast/Variable.h"

namespace cad {
namespace macro {
namespace ast {
namespace executable {
EntryFunction::EntryFunction(parser::Token token)
    : Function(std::move(token)) {
}

bool EntryFunction::operator==(const EntryFunction& other) const {
  return Function::operator==(other);
}
bool EntryFunction::operator!=(const EntryFunction& other) const {
  return !(*this == other);
}
}
}
}
}
