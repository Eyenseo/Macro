#ifndef cad_macro_ast_logic_If_h
#define cad_macro_ast_logic_If_h

#include "cad/macro/ast/logic/Condition.h"

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
namespace logic {
class If : public Condition {
protected:
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<Scope> true_scope;
  std::unique_ptr<Scope> false_scope;

  If() = default;
  If(const If& other);
  If(If&& other);
  If(parser::Token token);
  ~If();

  If& operator=(If other);

  friend void swap(If& first, If& second) {
    using std::swap;

    swap(static_cast<Condition&>(first), static_cast<Condition&>(second));
    swap(first.true_scope, second.true_scope);
    swap(first.false_scope, second.false_scope);
  }

  bool operator==(const If& other) const;
  bool operator!=(const If& other) const;

  friend std::ostream& operator<<(std::ostream& os, const If& ast) {
    ast.print_token(os, "If",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
