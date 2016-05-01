#ifndef cad_macro_ast_callable_Callable_h
#define cad_macro_ast_callable_Callable_h

#include "cad/macro/ast/AST.h"

#include <vector>

namespace cad {
namespace macro {
namespace ast {
struct Variable;
struct ValueProducer;
}
}
}

namespace cad {
namespace macro {
namespace ast {
namespace callable {
/**
 * @brief   The Callable struct represents all function class.
 *
 * @details The function calls can either be directed at ast::Function or a
 *          core::Command. The macro syntax is fun() / fun(foo:gun()) /
 *          fun(foo:gun(), bar:hun())
 */
struct Callable : public AST {
protected:
  /**
   * @brief  Pretty prints the internals of this struct
   *
   * @param  os    stream to print the internals to
   */
  void print_internals(IndentStream& os) const;

public:
  std::vector<std::pair<Variable, ValueProducer>> parameter;

  /**
   * @brief  Ctor
   */
  Callable();
  /**
   * @brief  CCtor
   */
  Callable(const Callable&);
  /**
   * @brief  MCtor
   */
  Callable(Callable&&);
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  Callable(parser::Token token);

  /**
   * @brief  Assignment operator
   *
   * @param  other  The other Operator to assign to this
   *
   * @return this
   */
  Callable& operator=(Callable other);

  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const Callable& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const Callable& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const Callable& ast) {
    ast.print_token(os, "Callable",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
/**
 * @brief  Swap function
 *
 * @param  first   Operator swapped with second
 * @param  second  Operator swapped with first
 */
void swap(Callable& first, Callable& second);
}
}
}
}
#endif
