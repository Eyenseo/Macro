#ifndef cad_macro_ast_loop_For_h
#define cad_macro_ast_loop_For_h

#include "cad/macro/ast/loop/While.h"
#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/Operator.h"
#include "cad/macro/ast/ValueProducer.h"
#include "cad/macro/ast/callable/Callable.h"

#include <core/variant.hpp>
#include <core/optional.hpp>

namespace cad {
namespace macro {
namespace ast {
namespace loop {
class For : public While {
protected:
  void print_internals(IndentStream& os) const;

public:
  ::core::optional<Variable> variable;
  ::core::optional<Operator> variable_init;
  ::core::optional<ValueProducer> operation;

  For();
  For(const For& other);
  For(For&& other);
  For(parser::Token token);
  ~For();

  For& operator=(For other);

  friend void swap(For& first, For& second) {
    // enable ADL
    using std::swap;

    swap(static_cast<While&>(first), static_cast<While&>(second));
    swap(first.variable, second.variable);
    swap(first.variable_init, second.variable_init);
    swap(first.operation, second.operation);
  }

  bool operator==(const For& other) const;
  bool operator!=(const For& other) const;

  friend std::ostream& operator<<(std::ostream& os, const For& ast) {
    ast.print_token(os, "For",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
