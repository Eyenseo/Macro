#ifndef cad_macro_ast_Scope_h
#define cad_macro_ast_Scope_h

#include "cad/macro/ast/AST.h"

#include "cad/macro/ast/Define.h"
#include "cad/macro/ast/Literal.h"
#include "cad/macro/ast/Operator.h"
#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/callable/Callable.h"
#include "cad/macro/ast/callable/Return.h"
#include "cad/macro/ast/logic/If.h"
#include "cad/macro/ast/loop/Break.h"
#include "cad/macro/ast/loop/Continue.h"
#include "cad/macro/ast/loop/DoWhile.h"
#include "cad/macro/ast/loop/For.h"
#include "cad/macro/ast/loop/While.h"

#include <eggs/variant.hpp>

#include <vector>

namespace cad {
namespace macro {
class IndentStream;
}
}

namespace cad {
namespace macro {
namespace ast {
/**
 * @brief   The Scope represents all scopes that are in the macro.
 * @details A all elements that are in between curly brackets ('{' and '}') are
 *          in a Scope. The interpreter::Stack realises this class, which means
 *          that these scopes work like C scopes.
 */
struct Scope : public AST {
private:
  using Callable = callable::Callable;
  using Return = callable::Return;
  using If = logic::If;
  using Break = loop::Break;
  using Continue = loop::Continue;
  using While = loop::While;
  using DoWhile = loop::DoWhile;
  using For = loop::For;

public:
  using Node =
      eggs::variant<Operator, Break, Continue, Callable, Define, DoWhile, For,
                    If, Literal<Literals::BOOL>, Literal<Literals::DOUBLE>,
                    Literal<Literals::INT>, Literal<Literals::STRING>, Return,
                    Scope, Variable, While>;

private:
  /**
   * @brief  Pretty prints the internals of this struct
   *
   * @param  os    stream to print the internals to
   */
  void print_internals(std::ostream& os) const;

public:
  std::vector<Node> nodes;

  /**
   * @brief  Ctor
   */
  Scope() = default;
  /**
   * @brief  Ctor
   *
   * @param  token  The token this node represents
   */
  Scope(parser::Token token);

  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const Scope& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The node to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const Scope& other) const;

  /**
   * @brief  Stream operator that will pretty print the node
   *
   * @param  os    The stream to print the node into
   * @param  ast   The node to print
   *
   * @return the input stream
   */
  friend std::ostream& operator<<(std::ostream& os, const Scope& ast) {
    ast.print_token(os, "Scope",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
#endif
