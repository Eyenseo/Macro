#ifndef cad_macro_ast_callable_EntryFunction_h
#define cad_macro_ast_callable_EntryFunction_h

#include "cad/macro/ast/callable/Function.h"

namespace cad {
namespace macro {
namespace ast {
namespace callable {
/**
 * @brief   The EntryFunction struct represents the main function declaration
 *          from the macro.
 *
 * @details The macro syntax is def main(){ ... } / def main(foo){ ... } / def
 *          main(foo, bar){ ... }
 */
struct EntryFunction : public Function {
public:
  /**
   * @brief  Ctor
   */
  EntryFunction() = default;
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  EntryFunction(parser::Token token);

  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const EntryFunction& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const EntryFunction& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const EntryFunction& ast) {
    ast.print_token(os, "EntryFunction",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
