#ifndef cad_macro_ast_loop_For_h
#define cad_macro_ast_loop_For_h

#include "cad/macro/ast/Define.h"
#include "cad/macro/ast/Operator.h"
#include "cad/macro/ast/ValueProducer.h"
#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/callable/Callable.h"
#include "cad/macro/ast/loop/While.h"

#include <eggs/variant.hpp>
#include <experimental/optional>

namespace cad {
namespace macro {
namespace ast {
namespace loop {
/**
 * @brief   The For struct represents the for elements in the macro.
 *
 * @details The syntax is for(;;){...} / for(def a = true;;){...} / for(a =
 *          true;;){...} / for(; a<0;){...} / for(; fun();){...} / for(;
 *          false;){...} / for(;;fun()){...} / for(;;false){...} / ...
 */
struct For : public While {
protected:
  /**
   * @brief  Pretty prints the internals of this struct
   *
   * @param  os    stream to print the internals to
   */
  void print_internals(IndentStream& os) const;

public:
  std::experimental::optional<Define> define;
  std::experimental::optional<ValueProducer> variable;
  std::experimental::optional<ValueProducer> operation;

  /**
   * @brief  Ctor
   */
  For() = default;
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  For(parser::Token token);

  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const For& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const For& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const For& ast) {
    ast.print_token(os, "For",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
