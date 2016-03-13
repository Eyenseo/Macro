#ifndef cad_macro_ast_executable_EntryFunction_h
#define cad_macro_ast_executable_EntryFunction_h

#include "cad/macro/ast/executable/Function.h"

namespace cad {
namespace macro {
namespace ast {
namespace executable {
class EntryFunction : public Function {
public:
  EntryFunction(parser::Token token);


  friend std::ostream& operator<<(std::ostream& os, const EntryFunction& ast) {
    ast.print_token(os, "EntryFunction",
                    [&ast](IndentStream& os) { ast.print_scope(os); });
    return os;
  }
};
}
}
}
}
#endif
