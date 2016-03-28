#ifndef cad_macro_ast_callable_Callable_h
#define cad_macro_ast_callable_Callable_h

#include "cad/macro/ast/AST.h"

#include <vector>

namespace cad {
namespace macro {
namespace ast {
class Variable;
class ValueProducer;
}
}
}

namespace cad {
namespace macro {
namespace ast {
namespace callable {
class Callable : public AST {
protected:
  void print_internals(IndentStream& os) const;

public:
  std::vector<std::pair<Variable, ValueProducer>> parameter;

  Callable();
  Callable(const Callable&);
  Callable(Callable&&);
  Callable(parser::Token token);
  ~Callable();

  Callable& operator=(Callable other);

  bool operator==(const Callable& other) const;
  bool operator!=(const Callable& other) const;

  friend std::ostream& operator<<(std::ostream& os, const Callable& ast) {
    ast.print_token(os, "Callable",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
void swap(Callable& first, Callable& second);
}
}
}
}
#endif
