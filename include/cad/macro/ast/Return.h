#ifndef cad_macro_ast_Return
#define cad_macro_ast_Return

#include "cad/macro/ast/AST.h"

#include <memory>

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
class Return : public AST {
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<ValueProducer> output;

  Return();
  Return(const Return&);
  Return(Return&&);
  Return(parser::Token token);
  ~Return();

  Return& operator=(Return other);

  friend void swap(Return& first, Return& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<AST&>(first), static_cast<AST&>(second));
    swap(first.output, second.output);
  }

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
