#ifndef cad_macro_ast_loop_Continue_h
#define cad_macro_ast_loop_Continue_h

#include "cad/macro/ast/AST.h"

#include <memory>

namespace cad {
namespace macro {
namespace ast {
struct ValueProducer;
}
}
}

namespace cad {
namespace macro {
namespace ast {
namespace loop {
/**
 * @brief   The Continue struct represents the continue elements in the macro.
 *
 * @details The macro syntax is continue;. Break can only be used in Loops like
 *          for and while.
 */
struct Continue : public AST {
public:
  /**
   * @brief  Ctor
   */
  Continue();
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  Continue(parser::Token token);

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const Continue& ast) {
    ast.print_token(os, "Continue");
    return os;
  }
};
}
}
}
}
#endif
