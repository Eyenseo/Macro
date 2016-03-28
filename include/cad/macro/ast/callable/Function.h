#ifndef cad_macro_ast_callable_Function_h
#define cad_macro_ast_callable_Function_h

#include "cad/macro/ast/AST.h"

#include <vector>
#include <memory>

namespace cad {
namespace macro {
namespace ast {
class Scope;
class Variable;
}
}
}

namespace cad {
namespace macro {
namespace ast {
namespace callable {
class Function : public AST {
protected:
  void print_internals(IndentStream& os) const;

public:
  std::vector<Variable> parameter;
  std::unique_ptr<Scope> scope;

  Function();
  Function(const Function&);
  Function(Function&&);
  Function(parser::Token token);
  ~Function();

  Function& operator=(Function other);

  friend void swap(Function& first, Function& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<AST&>(first), static_cast<AST&>(second));
    swap(first.scope, second.scope);
    swap(first.parameter, second.parameter);
  }

  bool operator==(const Function& other) const;
  bool operator!=(const Function& other) const;

  friend std::ostream& operator<<(std::ostream& os, const Function& ast) {
    ast.print_token(os, "Function",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
