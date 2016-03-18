#ifndef cad_macro_ast_Scope_h
#define cad_macro_ast_Scope_h

#include "cad/macro/ast/AST.h"

#include "cad/macro/ast/Define.h"
#include "cad/macro/ast/Return.h"
#include "cad/macro/ast/Operator.h"
#include "cad/macro/ast/executable/Function.h"
#include "cad/macro/ast/executable/EntryFunction.h"
#include "cad/macro/ast/executable/Executable.h"
#include "cad/macro/ast/logic/If.h"
#include "cad/macro/ast/loop/While.h"
#include "cad/macro/ast/loop/DoWhile.h"
#include "cad/macro/ast/loop/For.h"

#include <core/variant.hpp>

#include <vector>
#include <iomanip>
#include <iostream>

namespace cad {
namespace macro {
class IndentStream;
}
}

namespace cad {
namespace macro {
namespace ast {
class Scope : public AST {
  // TODO add all supported elements
  using EntryFunction = executable::EntryFunction;
  using Executable = executable::Executable;
  using Function = executable::Function;
  using If = logic::If;
  using While = loop::While;
  using DoWhile = loop::DoWhile;
  using For = loop::For;

public:
  using Node =
      core::variant<Define, EntryFunction, Executable, Function, Return, Scope,
                    UnaryOperator, BinaryOperator, If, While, DoWhile, For>;

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
