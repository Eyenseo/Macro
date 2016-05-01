#ifndef cad_macro_interpreter_Stack_h
#define cad_macro_interpreter_Stack_h

#include "cad/macro/ast/ValueProducer.h"
#include "cad/macro/ast/callable/Callable.h"
#include "cad/macro/ast/callable/Function.h"

#include <any.hpp>
#include <exception.h>

#include <algorithm>
#include <memory>
#include <vector>

namespace cad {
namespace macro {
namespace ast {
namespace callable {
struct Function;
}
}
}
}

namespace cad {
namespace macro {
namespace interpreter {
/**
 * @brief   The Stack manages the ast::Variable and ast::Function definitions of
 *          ast::Scope instances
 *
 * @details Each Stack instance is responsible for one ast::Scope. The Pointer
 *          to the parent Stack instance creates a non-navigateable tree where
 *          the cildren know their parents. This pointer is used to ask for
 *          definitions of ast::Variable or ast::Function.
 */
class Stack : public std::enable_shared_from_this<Stack> {
  template <typename T1, typename T2>
  using VecMap = std::vector<std::pair<T1, T2>>;
  using FunctionRef = std::reference_wrapper<const ast::callable::Function>;

  /**
   * @brief  Finds a given key in the VecMap
   *
   * @param  map   The map
   * @param  key   The key
   *
   * @return iterator to the found element or the end of the map
   */
  template <typename T1, typename T2>
  auto find(VecMap<T1, T2>& map, const T1& key) {
    return std::find_if(map.begin(), map.end(),
                        [&key](auto& pair) { return pair.first == key; });
  }
  /**
   * @brief  Finds a given key in the VecMap
   *
   * @param  map   The map
   * @param  key   The key
   *
   * @return iterator to the found element or the end of the map
   */
  template <typename T1, typename T2>
  auto find(const VecMap<T1, T2>& map, const T1& key) const {
    return std::find_if(map.begin(), map.end(),
                        [&key](const auto& pair) { return pair.first == key; });
  }

  /**
   * @brief  Finds function in the function vector
   *
   * @param  key   The key
   *
   * @return iterator to the found element or the end of the map
   */
  auto find_function(const ast::callable::Callable& key) const {
    return std::find_if(
        functions_.begin(), functions_.end(), [&key](const auto& fun) {
          if(fun.get().token.token == key.token.token &&
             fun.get().parameter.size() == key.parameter.size()) {
            for(const auto& fp : fun.get().parameter) {
              bool found = false;
              for(const auto& cp : key.parameter) {
                if(fp.token.token == cp.first.token.token) {
                  found = true;
                  break;
                }
              }
              if(!found) {
                return false;
              }
            }
          } else {
            return false;
          }
          return true;
        });
  }
  /**
   * @brief  Finds function in the function vector
   *
   * @param  key   The key
   *
   * @return true if found else false
   */
  bool exists_function(const ast::callable::Function& key) {
    return std::find_if(
               functions_.begin(), functions_.end(), [&key](const auto& fun) {
                 if(fun.get().token.token == key.token.token &&
                    fun.get().parameter.size() == key.parameter.size()) {
                   for(const auto& fp : fun.get().parameter) {
                     bool found = false;
                     for(const auto& cp : key.parameter) {
                       if(fp.token.token == cp.token.token) {
                         found = true;
                         break;
                       }
                     }
                     if(!found) {
                       return false;
                     }
                   }
                 } else {
                   return false;
                 }
                 return true;
               }) != functions_.end();
  }
  /**
   * @brief  Finds function in the function vector
   *
   * @param  key   The key
   *
   * @return true if found else false
   */
  auto exists_function(const ast::callable::Callable& key) {
    return find_function(key) != functions_.end();
  }
  /**
   * @brief  Finds function in the function vector by name only
   *
   * @param  name   The name
   *
   * @return true if found else false
   */
  auto exists_function(const std::string& name) {
    return functions_.end() !=
           std::find_if(functions_.begin(), functions_.end(),
                        [&name](const auto& fun) {
                          return fun.get().token.token == name;
                        });
  }
  /**
   * @brief  Finds variable in the variables vector by name only
   *
   * @param  name   The name
   *
   * @return true if found else false
   */
  auto exists_variable(const std::string& name) const {
    return find(variables_, name) != variables_.end();
  }

protected:
  std::shared_ptr<Stack> parent_;

  VecMap<std::string, linb::any> variables_;
  VecMap<std::string, std::reference_wrapper<linb::any>> aliases_;
  std::vector<FunctionRef> functions_;

public:
  enum class E {
    NOT_A_VARIABLE,
    NOT_A_FUNCTION,
    VARIABLE_EXISTS,
    FUNCTION_EXISTS
  };

  /**
   * @brief  Ctor
   */
  Stack();
  /**
   * @brief  Ctor
   *
   * @param  parent  The parent Stack
   */
  Stack(std::shared_ptr<Stack> parent);

  /**
   * @brief  Adds an alias / reference to a variable from another Stack
   *
   * @param  name      The name of the ast::Variable in this Stack
   * @param  variable  The any instance representing the ast::Variable
   *
   * @throws Exc<E,    E::VARIABLE_EXISTS>
   */
  void add_alias(std::string name, linb::any& variable);
  /**
   * @brief  Removes an alias / reference
   *
   * @param  name    The name of the ast::Variable to remove
   *
   * @throws Exc<E,  E::NOT_A_VARIABLE>
   */
  void remove_alias(std::string name);
  /**
   * @brief  Adds a any instance to represent the ast::Variable of given name
   *
   * @param  name    The name of the ast::Variable
   *
   * @throws Exc<E,  E::VARIABLE_EXISTS>
   */
  void add_variable(std::string name);
  /**
   * @brief  Adds a ast::Function to the function definitions
   *
   * @param  fun     The fun to 'define'
   *
   * @throws Exc<E,  E::FUNCTION_EXISTS>
   */
  void add_function(FunctionRef fun);

  /**
   * @brief  Checks if the given name is the name of an alias / reference
   *
   * @param  name  The name to check
   *
   * @return true if alias, false otherwise.
   */
  bool is_alias(const std::string& name) const;
  /**
   * @brief  Checks if this Stack owns an any instance that represents a
   *         ast::Variable
   *
   * @param  name  The name of the ast::Variable to check
   *
   * @return true if this stack has the any instance, false if it is a reference
   *         or from a parent Stack.
   */
  bool owns_variable(const std::string& name) const;

  /**
   * @brief  Checks if this Stack has access to an any instance representing a
   *         ast::Variable of given name
   *
   * @param  name  The name of the ast::Variable to check
   *
   * @return True if has variable, False otherwise.
   */
  bool has_variable(const std::string& name) const;
  /**
   * @brief  Checks if this Stack has access to a function of given name.
   *
   * @param  call  The callable to check
   *
   * @return True if has function, False otherwise.
   */
  bool has_function(const ast::callable::Callable& call) const;

  /**
   * @brief  parent
   *
   * @return parent
   */
  std::shared_ptr<Stack> parent() const;

  /**
   * @brief  Function to access an any instance that represents a ast::Variable
   *
   * @param  name    The name of the ast::Variable to access
   * @param  fun     The function to execute if the variable exists
   *
   * @tparam FUN     Lambda function [](linb::any& var) {...}
   *
   * @throws Exc<E,  E::NOT_A_VARIABLE>
   */
  template <typename FUN,
            typename std::enable_if<
                std::is_same<std::result_of_t<FUN(linb::any&)>, void>::value,
                bool>::type = false>
  void variable(const std::string& name, FUN fun) {
    auto alias_it = find(aliases_, name);

    if(alias_it != aliases_.end()) {
      fun(alias_it->second);
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
        fun(own_it->second);
      }
    }
  }
  /**
   * @brief  Function to access a ast::Function
   *
   * @param  call    The ast:callable::Callable to get the function for
   * @param  fun     The function to execute if the function exists
   *
   * @tparam FUN     Lambda function [&](const Function& fun, auto stack) {...}
   *                 the stack will be the pointer the function was defined in
   *
   * @throws Exc<E,  E::NOT_A_FUNCTION>
   */
  template <
      typename FUN,
      typename std::enable_if<
          std::is_same<std::result_of_t<FUN(const ast::callable::Function&,
                                            std::shared_ptr<Stack>)>,
                       void>::value,
          bool>::type = false>
  void function(const ast::callable::Callable& call, FUN fun) {
    auto it = find_function(call);

    if(it == functions_.end()) {
      if(parent_) {
        parent_->function(call, std::move(fun));
      } else {
        bool first = true;

        Exc<E, E::NOT_A_FUNCTION> e(__FILE__, __LINE__, "Not a function");
        e << "The is no function '" << call.token.token
          << "', with the parameter signature '(";
        for(const auto& p : call.parameter) {
          if(first) {
            e << p.first.token.token;
            first = false;
          } else {
            e << ", " << p.first.token.token;
          }
        }
        e << ")' in this or any parent stacks.";
        throw e;
      }
    } else {
      fun(*it, shared_from_this());
    }
  }
};
}
}
}
#endif
