#ifndef cad_macro_ast_loop_Break_h
#define cad_macro_ast_loop_Break_h

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
 * @brief   The Break struct represents the break elements in the macro.
 *
 * @details The macro syntax is break;. Break can only be used in Loops like for
 *          and while.
 */
struct Break : public AST {
public:
  /**
   * @brief  Ctor
   */
  Break();
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  Break(parser::Token token);

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const Break& ast) {
    ast.print_token(os, "Break");
    return os;
  }
};
}
}
}
}
#endif
