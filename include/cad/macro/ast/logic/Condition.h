#ifndef cad_macro_ast_logic_Condition_h
#define cad_macro_ast_logic_Condition_h

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
namespace logic {
class Condition : public AST {
protected:
  void print_internals(IndentStream& os) const;

public:
  std::unique_ptr<ValueProducer> condition;

  Condition();
  Condition(const Condition&);
  Condition(Condition&&);
  Condition(parser::Token token);
  ~Condition();

  Condition& operator=(Condition other);

  friend void swap(Condition& first, Condition& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<AST&>(first), static_cast<AST&>(second));
    swap(first.condition, second.condition);
  }

  bool operator==(const Condition& other) const;
  bool operator!=(const Condition& other) const;

  friend std::ostream& operator<<(std::ostream& os, const Condition& ast) {
    ast.print_token(os, "Condition",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
