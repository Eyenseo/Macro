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
  using FunctionRef = std::reference_wrapper<const ast::callable::Function>;

  template <typename T1, typename T2>
  auto find(VecMap<T1, T2>& map, const T1& key) {
    return std::find_if(map.begin(), map.end(),
                        [&key](auto& pair) { return pair.first == key; });
  }
  template <typename T1, typename T2>
  auto find(const VecMap<T1, T2>& map, const T1& key) const {
    return std::find_if(map.begin(), map.end(),
                        [&key](const auto& pair) { return pair.first == key; });
  }
  template <typename T1, typename T2>
  auto exists(const VecMap<T1, T2>& map, const T1& key) const {
    return find(map, key) != map.end();
  }

protected:
  std::shared_ptr<Stack> parent_;

  VecMap<std::string, ::core::any> variables_;
  VecMap<std::string, std::reference_wrapper<::core::any>> aliases_;
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

  void add_alias(std::string name, ::core::any& variable);
  void remove_alias(std::string name);
  void add_variable(std::string name);
  void add_function(std::string name, FunctionRef fun);

  bool is_alias(const std::string& name) const;
  bool owns_variable(const std::string& name) const;

  bool has_variable(const std::string& name) const;
  bool has_function(const std::string& name) const;

  std::shared_ptr<Stack> parent() const;

  template <typename FUN>
  auto variable(const std::string& name, FUN fun)
      -> decltype(std::result_of_t<FUN(::core::any&)>()) {
    auto alias_it = find(aliases_, name);

    if(alias_it != aliases_.end()) {
      return fun(alias_it->second);
    } else {
      auto own_it = find(variables_, name);

      if(own_it == variables_.end()) {
        if(parent_) {
          return parent_->variable(name, std::move(fun));
        } else {
          Exc<E, E::NOT_A_VARIABLE> e(__FILE__, __LINE__, "Not a variable");
          e << "The is no variable '" << name
            << "' in this or any parent stacks.";
          throw e;
        }
      } else {
        return fun(own_it->second);
      }
    }
  }

  template <typename FUN>
  auto function(const std::string& name, FUN fun)
      -> decltype(std::result_of_t<FUN(const ast::callable::Function&,
                                       std::shared_ptr<Stack>)>()) {
    auto it = find(functions_, name);

    if(it == functions_.end()) {
      if(parent_) {
        return parent_->function(name, std::move(fun));
      } else {
        Exc<E, E::NOT_A_FUNCTION> e(__FILE__, __LINE__, "Not a function");
        e << "The is no function '" << name
          << "' in this or any parent stacks.";
        throw e;
      }
    } else {
      return fun(it->second, shared_from_this());
    }
  }
};
}
}
}
#endif
