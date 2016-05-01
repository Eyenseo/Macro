#ifndef cad_macro_ast_logic_If_h
#define cad_macro_ast_logic_If_h

#include "cad/macro/ast/logic/Condition.h"

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
namespace logic {
/**
 * @brief   The If struct represents all if/else control elements in the macro
 *
 * @details The macro syntax is if(ValueProducer){...} /
 *          if(ValueProducer){...} else {...}
 */
struct If : public Condition {
protected:
  /**
   * @brief  Pretty prints the internals of this struct
   *
   * @param  os    stream to print the internals to
   */
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<Scope> true_scope;
  std::unique_ptr<Scope> false_scope;

  /**
   * @brief  Ctor
   */
  If() = default;
  /**
   * @brief  CCtor
   */
  If(const If& other);
  /**
   * @brief  MCtor
   */
  If(If&& other);
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  If(parser::Token token);
  /**
   * @brief  Dtor
   */
  ~If();

  /**
   * @brief  Swap function
   *
   * @param  first   Operator swapped with second
   * @param  second  Operator swapped with first
   */
  friend void swap(If& first, If& second) {
    using std::swap;

    swap(static_cast<Condition&>(first), static_cast<Condition&>(second));
    swap(first.true_scope, second.true_scope);
    swap(first.false_scope, second.false_scope);
  }

  /**
   * @brief  Assignment operator
   *
   * @param  other  The other Operator to assign to this
   *
   * @return this
   */
  If& operator=(If other);
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const If& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const If& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const If& ast) {
    ast.print_token(os, "If",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
