#ifndef cad_macro_ast_executable_Executable_h
#define cad_macro_ast_executable_Executable_h

#include "cad/macro/ast/AST.h"
#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/executable/Executable.h"

#include <core/variant.hpp>

#include <vector>

namespace cad {
namespace macro {
namespace ast {
namespace executable {
class Executable : public AST {
protected:
  void print_internals(IndentStream& os) const;

public:
  using Input = core::variant<Variable, Executable>;
  std::vector<Input> parameter;

  Executable() = default;
  Executable(parser::Token token);

  bool operator==(const Executable& other) const;
  bool operator!=(const Executable& other) const;

  friend std::ostream& operator<<(std::ostream& os, const Executable& ast) {
    ast.print_token(os, "Executable",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
