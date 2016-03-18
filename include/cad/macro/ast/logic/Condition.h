#ifndef cad_macro_ast_logic_Condition_h
#define cad_macro_ast_logic_Condition_h

#include "cad/macro/ast/AST.h"
#include "cad/macro/ast/Operator.h"

namespace cad {
namespace macro {
namespace ast {
namespace logic {
class Condition : public AST {
protected:
  void print_internals(IndentStream& os) const;

public:
  core::optional<core::variant<executable::Executable, Variable, UnaryOperator,
                               BinaryOperator>> condition;

  Condition() = default;
  Condition(parser::Token token);

  bool operator==(const Condition& other) const;
  bool operator!=(const Condition& other) const;

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
