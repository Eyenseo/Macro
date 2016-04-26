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
#include "cad/macro/ast/loop/DoWhile.h"
#include "cad/macro/ast/loop/For.h"
#include "cad/macro/ast/loop/While.h"

#include <core/variant.hpp>

#include <vector>

namespace cad {
namespace macro {
class IndentStream;
}
}

namespace cad {
namespace macro {
namespace ast {
class Scope : public AST {
  using Callable = callable::Callable;
  using Return = callable::Return;
  using If = logic::If;
  using Break = loop::Break;
  using While = loop::While;
  using DoWhile = loop::DoWhile;
  using For = loop::For;

public:
  using Node =
      ::core::variant<Operator, Break, Callable, Define, DoWhile, For, If,
                      Literal<Literals::BOOL>, Literal<Literals::DOUBLE>,
                      Literal<Literals::INT>, Literal<Literals::STRING>, Return,
                      Scope, Variable, While>;

private:
  void print_internals(std::ostream& os) const;
  void print_var(IndentStream& os, const Node& var) const;

public:
  std::vector<Node> nodes;

  Scope() = default;
  Scope(parser::Token token);

  bool operator==(const Scope& other) const;
  bool operator!=(const Scope& other) const;

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
