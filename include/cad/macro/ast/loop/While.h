#ifndef cad_macro_ast_loop_While_h
#define cad_macro_ast_loop_While_h

#include "cad/macro/ast/AST.h"
#include "cad/macro/ast/logic/Condition.h"

#include <memory>

namespace cad {
namespace macro {
namespace ast {
struct Scope;
}
}
}

namespace cad {
namespace macro {
namespace ast {
namespace loop {
/**
 * @brief   The While struct represents the do-while elements in the macro.
 *
 * @details The macro syntax is while(Condition){...}.
 */
struct While : public logic::Condition {
protected:
  /**
   * @brief  Pretty prints the internals of this struct
   *
   * @param  os    stream to print the internals to
   */
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<Scope> scope;

  /**
   * @brief  Ctor
   */
  While();
  /**
   * @brief  CCtor
   */
  While(const While& other);
  /**
   * @brief  MCtor
   */
  While(While&& other);
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  While(parser::Token token);
   /**
   * @brief  Dtor
   */
  ~While();

  /**
   * @brief  Swap function
   *
   * @param  first   Operator swapped with second
   * @param  second  Operator swapped with first
   */
  friend void swap(While& first, While& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<Condition&>(first), static_cast<Condition&>(second));
    swap(first.scope, second.scope);
  }
  /**
   * @brief  Assignment operator
   *
   * @param  other  The other Operator to assign to this
   *
   * @return this
   */
  While& operator=(While other);
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const While& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const While& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const While& ast) {
    ast.print_token(os, "While",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
