#ifndef cad_macro_ast_callable_Function_h
#define cad_macro_ast_callable_Function_h

#include "cad/macro/ast/AST.h"

#include <vector>
#include <memory>

namespace cad {
namespace macro {
namespace ast {
struct Scope;
struct Variable;
}
}
}

namespace cad {
namespace macro {
namespace ast {
namespace callable {
/**
 * @brief   The function struct represents the function declarations from the
 *          macro.
 *
 * @details The macro syntax is def fun(){ ... } / def fun(foo){ ... } / def
 *          fun(foo, bar){ ... }
 */
struct Function : public AST {
protected:
  /**
   * @brief  Pretty prints the internals of this struct
   *
   * @param  os    stream to print the internals to
   */
  void print_internals(IndentStream& os) const;

public:
  std::vector<Variable> parameter;
  std::unique_ptr<Scope> scope;

  /**
   * @brief  Ctor
   */
  Function();
  /**
   * @brief  CCtor
   */
  Function(const Function&);
  /**
   * @brief  MCtor
   */
  Function(Function&&);
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  Function(parser::Token token);
  /**
   * @brief  Dtor
   */
  ~Function();

  /**
   * @brief  Swap function
   *
   * @param  first   Operator swapped with second
   * @param  second  Operator swapped with first
   */
  friend void swap(Function& first, Function& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<AST&>(first), static_cast<AST&>(second));
    swap(first.scope, second.scope);
    swap(first.parameter, second.parameter);
  }
  /**
   * @brief  Assignment operator
   *
   * @param  other  The other Operator to assign to this
   *
   * @return this
   */
  Function& operator=(Function other);
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const Function& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const Function& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const Function& ast) {
    ast.print_token(os, "Function",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
