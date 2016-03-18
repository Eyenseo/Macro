#ifndef cad_macro_ast_Return
#define cad_macro_ast_Return

#include "cad/macro/ast/AST.h"
#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/executable/Executable.h"


#include <core/variant.hpp>
#include <core/optional.hpp>


namespace cad {
namespace macro {
namespace ast {
class Return : public AST {
  using Output = core::variant<Variable, executable::Executable>;

  void print_internals(IndentStream& os) const;

public:
  core::optional<Output> output;

  Return() = default;
  Return(parser::Token token);

  bool operator==(const Return& other) const;
  bool operator!=(const Return& other) const;

  friend std::ostream& operator<<(std::ostream& os, const Return& ast) {
    ast.print_token(os, "Return",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
#endif
