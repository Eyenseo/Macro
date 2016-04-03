#include "cad/macro/interpreter/Stack.h"

namespace cad {
namespace macro {
namespace interpreter {
namespace {
[[noreturn]] void throw_function_exists(const char* const file,
                                        const size_t line,
                                        const std::string& name) {
  Exc<Stack::E, Stack::E::FUNCTION_EXISTS> e(file, line, "The function exists");
  e << "The function '" << name << "' already exists.";
  throw e;
}
[[noreturn]] void throw_variable_exists(const char* const file,
                                        const size_t line,
                                        const std::string& name) {
  Exc<Stack::E, Stack::E::VARIABLE_EXISTS> e(file, line, "The variable exists");
  e << "The variable '" << name << "' already exists.";
  throw e;
}
}

Stack::Stack() {
}
Stack::Stack(std::shared_ptr<Stack> parent)
    : parent_(std::move(parent)) {
}

std::shared_ptr<Stack> Stack::parent() const {
  return parent_;
}

void Stack::add_variable(std::string name) {
  if(exists(functions_, name)) {

  } else if(exists(variables_, name)) {
  }
  variables_.emplace_back(std::move(name), ::core::any());
}

void Stack::add_function(std::string name, FunctionRef fun) {
  if(exists(functions_, name)) {
    throw_function_exists(__FILE__, __LINE__, name);
  } else if(exists(variables_, name)) {
    throw_variable_exists(__FILE__, __LINE__, name);
  }
  functions_.emplace_back(std::move(name), std::move(fun));
}

void Stack::add_alias(std::string name, ::core::any& variable) {
  if(exists(functions_, name)) {
    throw_function_exists(__FILE__, __LINE__, name);
  } else if(exists(variables_, name)) {
    throw_variable_exists(__FILE__, __LINE__, name);
  }
  aliases_.emplace_back(std::move(name), variable);
}
void Stack::remove_alias(std::string alias) {
  auto it = find(aliases_, alias);

  if(it != aliases_.end()) {
    aliases_.erase(it);
  }
}
bool Stack::is_alias(const std::string& name) const {
  auto it = find(aliases_, name);

  return it != aliases_.end();
}
bool Stack::owns_variable(const std::string& name) const {
  auto it = find(variables_, name);

  return it != variables_.end();
}

bool Stack::has_variable(const std::string& name) const {
  auto alias_it = find(aliases_, name);

  if(alias_it != aliases_.end()) {
    return true;
  } else {
    auto own_it = find(variables_, name);

    if(own_it == variables_.end()) {
      if(parent_) {
        return parent_->has_variable(name);
      }
      return false;
    } else {
      return true;
    }
  }
}

bool Stack::has_function(const std::string& name) const {
  auto it = find(functions_, name);

  if(it == functions_.end()) {
    if(parent_) {
      return parent_->has_function(name);
    }
    return false;
  } else {
    return true;
  }
}
}
}
}
