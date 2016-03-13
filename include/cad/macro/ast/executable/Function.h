#ifndef cad_macro_ast_executable_Function_h
#define cad_macro_ast_executable_Function_h

#include "cad/macro/ast/AST.h"

#include <memory>

namespace cad {
namespace macro {
namespace ast {
class Scope;
}
}
}

namespace cad {
namespace macro {
namespace ast {
namespace executable {
class Function : public AST {
  std::shared_ptr<Scope> scope_;

protected:
  void print_scope(IndentStream& os) const;

public:
  Function(parser::Token token);

  void set_scope(Scope scope);
  const Scope& get_scope() const;

  friend std::ostream& operator<<(std::ostream& os, const Function& ast) {
    ast.print_token(os, "Function",
                    [&ast](IndentStream& os) { ast.print_scope(os); });
    return os;
  }
};
}
}
}
}
#endif
