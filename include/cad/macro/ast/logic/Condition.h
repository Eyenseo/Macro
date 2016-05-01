#ifndef cad_macro_ast_logic_Condition_h
#define cad_macro_ast_logic_Condition_h

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
namespace logic {
/**
 * @brief  The Condition struct is a convenience struct that wraps a
 *         ValueProducer
 */
struct Condition : public AST {
protected:
  /**
   * @brief  Pretty prints the internals of this struct
   *
   * @param  os    stream to print the internals to
   */
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<ValueProducer> condition;

  /**
   * @brief  Ctor
   */
  Condition();
  /**
   * @brief  CCtor
   */
  Condition(const Condition&);
  /**
   * @brief  MCtor
   */
  Condition(Condition&&);
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  Condition(parser::Token token);
  /**
   * @brief  Dtor
   */
  ~Condition();

  /**
   * @brief  Swap function
   *
   * @param  first   Operator swapped with second
   * @param  second  Operator swapped with first
   */
  friend void swap(Condition& first, Condition& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<AST&>(first), static_cast<AST&>(second));
    swap(first.condition, second.condition);
  }

  /**
   * @brief  Assignment operator
   *
   * @param  other  The other Operator to assign to this
   *
   * @return this
   */
  Condition& operator=(Condition other);
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const Condition& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const Condition& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const Condition& ast) {
    ast.print_token(os, "Condition",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
