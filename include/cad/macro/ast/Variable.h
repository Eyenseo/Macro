#ifndef cad_macro_ast_Variable_h
#define cad_macro_ast_Variable_h

#include "cad/macro/ast/AST.h"

namespace cad {
namespace macro {
namespace ast {
class Variable : public AST {
public:
  Variable() = default;
  Variable(parser::Token token);

  friend std::ostream& operator<<(std::ostream& os, const Variable& ast) {
    ast.print_token(os, "Variable");
    return os;
  }
};
}
}
}
#endif
