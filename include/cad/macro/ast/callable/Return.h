#ifndef cad_macro_ast_callable_Return_h
#define cad_macro_ast_callable_Return_h

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
namespace callable {
/**
 * @brief   The Return struct represents all return statements.
 *
 * @details The macro syntax is return foo / return fun() / return true == false
 */
struct Return : public AST {
  /**
   * @brief  Pretty prints the internals of this struct
   *
   * @param  os    stream to print the internals to
   */
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<ValueProducer> output;

  /**
   * @brief  Ctor
   */
  Return();
  /**
   * @brief  CCtor
   */
  Return(const Return&);
  /**
   * @brief  MCtor
   */
  Return(Return&&);
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  Return(parser::Token token);

  /**
   * @brief  Swap function
   *
   * @param  first   Operator swapped with second
   * @param  second  Operator swapped with first
   */
  friend void swap(Return& first, Return& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<AST&>(first), static_cast<AST&>(second));
    swap(first.output, second.output);
  }
  /**
   * @brief  Assignment operator
   *
   * @param  other  The other Operator to assign to this
   *
   * @return this
   */
  Return& operator=(Return other);
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const Return& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const Return& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const Return& ast) {
    ast.print_token(os, "Return",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
