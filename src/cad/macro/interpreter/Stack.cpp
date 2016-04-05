#include "cad/macro/interpreter/Stack.h"

#include "cad/macro/ast/callable/Function.h"
#include "cad/macro/ast/Variable.h"

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
[[noreturn]] void throw_function_exists(const char* const file,
                                        const size_t line,
                                        const ast::callable::Function& fun) {
  bool first = true;
  Exc<Stack::E, Stack::E::FUNCTION_EXISTS> e(file, line, "The function exists");
  e << "The function '" << fun.token.token
    << "', with the parameter signature '(";
  for(const auto& p : fun.parameter) {
    if(first) {
      e << p.token.token;
      first = false;
    } else {
      e << ", " << p.token.token;
    }
  }
  e << ")' already exists.";
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
  if(exists_function(name)) {
    throw_function_exists(__FILE__, __LINE__, name);
  } else if(exists_variable(name)) {
    throw_variable_exists(__FILE__, __LINE__, name);
  }
  variables_.emplace_back(std::move(name), ::core::any());
}

void Stack::add_function(FunctionRef fun) {
  if(exists_function(fun.get())) {
    throw_function_exists(__FILE__, __LINE__, fun);
  } else if(exists_variable(fun.get().token.token)) {
    throw_variable_exists(__FILE__, __LINE__, fun.get().token.token);
  }
  functions_.emplace_back(std::move(fun));
}

void Stack::add_alias(std::string name, ::core::any& variable) {
  if(exists_function(name)) {
    throw_function_exists(__FILE__, __LINE__, name);
  } else if(exists_variable(name)) {
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

bool Stack::has_function(const ast::callable::Callable& call) const {
  auto it = find_function(call);

  if(it == functions_.end()) {
    if(parent_) {
      return parent_->has_function(call);
    }
    return false;
  } else {
    return true;
  }
}
}
}
}
