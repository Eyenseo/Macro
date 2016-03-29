#include "cad/macro/interpreter/Stack.h"

namespace cad {
namespace macro {
namespace interpreter {
Stack::Stack() {
}
Stack::Stack(std::shared_ptr<Stack> parent)
    : parent_(std::move(parent)) {
}

std::shared_ptr<Stack> Stack::parent() const {
  return parent_;
}

void Stack::add_variable(std::string name) {
  if(exists(variables_, name)) {
    Exc<E, E::VARIABLE_EXISTS> e(__FILE__, __LINE__, "The variable exists");
    e << "The variable '" << name << "' already exists.";
    throw e;
  } else {
    variables_.push_back(std::make_pair(std::move(name), core::any()));
  }
}

void Stack::add_function(std::string name, FunctionRef fun) {
  if(exists(functions_, name)) {
    Exc<E, E::FUNCTION_EXISTS> e(__FILE__, __LINE__, "The function exists");
    e << "The function '" << name << "' already exists.";
    throw e;
  } else {
    functions_.push_back(std::make_pair(std::move(name), std::move(fun)));
  }
}
}
}
}
