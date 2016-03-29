#ifndef cad_macro_interpreter_Stack_h
#define cad_macro_interpreter_Stack_h

#include "cad/macro/ast/callable/Function.h"

#include <exception.h>

#include <core/any.hpp>

#include <memory>
#include <vector>

namespace cad {
namespace macro {
namespace interpreter {
class Stack : public std::enable_shared_from_this<Stack> {
  template <typename T1, typename T2>
  using VecMap = std::vector<std::pair<T1, T2>>;
  using FunctionRef = std::reference_wrapper<ast::callable::Function>;


  template <typename T1, typename T2>
  auto find(VecMap<T1, T2>& map, const T1& key) {
    return std::find_if(map.begin(), map.end(),
                        [&key](auto& pair) { return pair.first == key; });
  }
  template <typename T1, typename T2>
  auto find(const VecMap<T1, T2>& map, const T1& key) {
    return std::find_if(map.begin(), map.end(),
                        [&key](const auto& pair) { return pair.first == key; });
  }
  template <typename T1, typename T2>
  auto exists(const VecMap<T1, T2>& map, const T1& key) {
    return find(map, key) != map.end();
  }

protected:
  std::shared_ptr<Stack> parent_;

  VecMap<std::string, core::any> variables_;
  VecMap<std::string, FunctionRef> functions_;

public:
  enum class E {
    NOT_A_VARIABLE,
    NOT_A_FUNCTION,
    VARIABLE_EXISTS,
    FUNCTION_EXISTS
  };

  Stack();
  Stack(std::shared_ptr<Stack> parent);

  void add_variable(std::string name);
  void add_function(std::string name, FunctionRef fun);

  std::shared_ptr<Stack> parent() const;

  template <typename FUN>
  void variable(const std::string& name, FUN fun) {
    auto it = find(variables_, name);

    if(it == variables_.end()) {
      if(parent_) {
        parent_->variable(name, std::move(fun));
      } else {
        Exc<E, E::NOT_A_VARIABLE> e(__FILE__, __LINE__, "Not a variable");
        e << "The is no variable '" << name
          << "' in this or any parent stacks.";
        throw e;
      }
    } else {
      fun(it->second);
    }
  }

  template <typename FUN>
  void function(const std::string& name, FUN fun) {
    auto it = find(functions_, name);

    if(it == functions_.end()) {
      if(parent_) {
        parent_->function(name, std::move(fun));
      } else {
        Exc<E, E::NOT_A_FUNCTION> e(__FILE__, __LINE__, "Not a function");
        e << "The is no function '" << name
          << "' in this or any parent stacks.";
        throw e;
      }
    } else {
      fun(it->second);
    }
  }
};
}
}
}
#endif
