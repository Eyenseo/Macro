#ifndef cad_macro_ast_loop_While_h
#define cad_macro_ast_loop_While_h

#include "cad/macro/ast/AST.h"
#include "cad/macro/ast/logic/Condition.h"

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
namespace loop {
class While : public AST {
protected:
  void print_internals(IndentStream& os) const;

public:
  logic::Condition condition;
  std::unique_ptr<Scope> scope;

  While();
  While(const While& other);
  While(While&& other);
  While(parser::Token token);
  ~While();

  While& operator=(While other);

  friend void swap(While& first, While& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<AST&>(first), static_cast<AST&>(second));
    swap(first.condition, second.condition);
    swap(first.scope, second.scope);
  }

  bool operator==(const While& other) const;
  bool operator!=(const While& other) const;

  friend std::ostream& operator<<(std::ostream& os, const While& ast) {
    ast.print_token(os, "While",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
