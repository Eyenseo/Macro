#ifndef cad_macro_ast_Scope_h
#define cad_macro_ast_Scope_h

#include "cad/macro/ast/AST.h"

#include "cad/macro/ast/Define.h"
#include "cad/macro/ast/executable/Function.h"
#include "cad/macro/ast/executable/EntryFunction.h"

#include <core/variant.hpp>

#include <vector>
#include <iomanip>
#include <iostream>

namespace cad {
namespace macro {
class IndentStream;
}
}

namespace cad {
namespace macro {
namespace ast {
class Scope : public AST {
  // TODO add all supported elements
  using Function = executable::Function;
  using EntryFunction = executable::EntryFunction;
  using Var = core::variant<Scope, Define, Function, EntryFunction>;
  template <typename T>
  using valid_type =
      typename std::enable_if<std::is_base_of<AST, T>::value &&
                                  (std::is_same<Scope, T>::value ||
                                   std::is_same<Function, T>::value ||
                                   std::is_same<EntryFunction, T>::value ||
                                   std::is_same<Define, T>::value),
                              bool>::type;

  std::vector<Var> nodes_;

  static void print_scope(std::ostream& os, const Scope& scope);
  static void print_var(IndentStream& os, const Var& var);

public:
  Scope(parser::Token token);

  friend bool operator==(const Scope& first, const Scope& second) {
    if(&first == &second) {
      return true;
    } else {
      if(first.token_ == second.token_) {
        return first.nodes_ == second.nodes_;
      } else {
        return false;
      }
    }
  }

  template <typename T, valid_type<T> = true>
  void append(T&& ele) {
    nodes_.push_back(std::forward<T>(ele));
  }

  friend std::ostream& operator<<(std::ostream& os, const Scope& scope) {
    print_scope(os, scope);
    return os;
  }
};
bool operator==(const Scope& first, const AST& second);
}
}
}
#endif
