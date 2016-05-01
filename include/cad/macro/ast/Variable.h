#ifndef cad_macro_ast_Variable_h
#define cad_macro_ast_Variable_h

#include "cad/macro/ast/AST.h"

namespace cad {
namespace macro {
namespace ast {
/**
 * @brief  The Variable class represents all variables in the macro.
 */
struct Variable : public AST {
public:
  /**
   * @brief  Ctor
   */
  Variable() = default;
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  Variable(parser::Token token);

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const Variable& ast) {
    ast.print_token(os, "Variable");
    return os;
  }
};
}
}
}
#endif
