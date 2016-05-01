#ifndef cad_macro_ast_loop_DoWhile_h
#define cad_macro_ast_loop_DoWhile_h

#include "cad/macro/ast/loop/While.h"

namespace cad {
namespace macro {
namespace ast {
namespace loop {
/**
 * @brief   The DoWhile struct represents the do-while elements in the macro.
 *
 * @details The macro syntax is do{...}while(Condition);.
 */
struct DoWhile : public While {
public:
  /**
   * @brief  Ctor
   */
  DoWhile() = default;
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  DoWhile(parser::Token token);

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const DoWhile& ast) {
    ast.print_token(os, "DoWhile",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
