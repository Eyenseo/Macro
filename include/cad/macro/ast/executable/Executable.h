#ifndef cad_macro_ast_executable_Executable_h
#define cad_macro_ast_executable_Executable_h

#include "cad/macro/ast/AST.h"

#include <vector>

namespace cad {
namespace macro {
namespace ast {
class ValueProducer;
}
}
}

namespace cad {
namespace macro {
namespace ast {
namespace executable {
class Executable : public AST {
protected:
  void print_internals(IndentStream& os) const;

public:
  std::vector<ValueProducer> parameter;

  Executable();
  Executable(const Executable&);
  Executable(Executable&&);
  Executable(parser::Token token);
  ~Executable();

  Executable& operator=(Executable other);

  friend void swap(Executable& first, Executable& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<AST&>(first), static_cast<AST&>(second));
    swap(first.parameter, second.parameter);
  }

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
