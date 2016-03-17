#ifndef cad_macro_ast_executable_Function_h
#define cad_macro_ast_executable_Function_h

#include "cad/macro/ast/executable/Executable.h"

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
namespace executable {
class Function : public Executable {
protected:
  void print_internals(IndentStream& os) const;

public:
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

    swap(dynamic_cast<Executable&>(first), dynamic_cast<Executable&>(second));
    swap(first.scope, second.scope);
  }

  bool operator==(const Function& other) const;
  bool operator!=(const Function& other) const;

  friend std::ostream& operator<<(std::ostream& os, const Function& ast) {
    ast.print_token(os, "Function",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
bool operator==(const Function& first, const AST& second);
}
}
}
}
#endif
