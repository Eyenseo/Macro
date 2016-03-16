#ifndef cad_macro_ast_executable_EntryFunction_h
#define cad_macro_ast_executable_EntryFunction_h

#include "cad/macro/ast/executable/Function.h"

namespace cad {
namespace macro {
namespace ast {
namespace executable {
class EntryFunction : public Function {
public:
  EntryFunction() = default;
  EntryFunction(parser::Token token);

  bool operator==(const EntryFunction& other) const;
  bool operator!=(const EntryFunction& other) const;

  friend std::ostream& operator<<(std::ostream& os, const EntryFunction& ast) {
    ast.print_token(os, "EntryFunction",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
bool operator==(const EntryFunction& first, const AST& second);
}
}
}
}
#endif
