#ifndef cad_macro_ast_Define_h
#define cad_macro_ast_Define_h

#include "cad/macro/ast/AST.h"
#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/callable/Function.h"
#include "cad/macro/ast/callable/EntryFunction.h"

#include "cad/macro/IndentStream.h"

#include <eggs/variant.hpp>

namespace cad {
namespace macro {
namespace ast {
/**
 * @brief   The Define struct represents all definitions done in the macro.
 *
 * @details `var` and `def` are represented by this struct. If there is an
 *          assignment after the definition it will be represented by an
 *          Operator and not Define.
 */
struct Define : public AST {
private:
  using Function = callable::Function;
  using EntryFunction = callable::EntryFunction;
  using Definition = eggs::variant<Function, EntryFunction, Variable>;

  /**
   * @brief  Pretty prints the internals of this struct
   *
   * @param  os    stream to print the internals to
   */
  void print_internals(IndentStream& os) const;

public:
  Definition definition;

  /**
   * @brief  Ctor
   */
  Define() = default;
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  Define(parser::Token token);

  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const Define& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const Define& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const Define& ast) {
    ast.print_token(os, "Define",
                    [&ast](IndentStream& os) { ast.print_internals(os); });

    return os;
  }
};
}
}
}
#endif
