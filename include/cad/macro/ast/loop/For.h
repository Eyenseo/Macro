#ifndef cad_macro_ast_loop_For_h
#define cad_macro_ast_loop_For_h

#include "cad/macro/ast/Define.h"
#include "cad/macro/ast/Operator.h"
#include "cad/macro/ast/ValueProducer.h"
#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/callable/Callable.h"
#include "cad/macro/ast/loop/While.h"

#include <eggs/variant.hpp>
#include <experimental/optional>

namespace cad {
namespace macro {
namespace ast {
namespace loop {
class For : public While {
protected:
  void print_internals(IndentStream& os) const;

public:
  std::experimental::optional<Define> define;
  std::experimental::optional<ValueProducer> variable;
  std::experimental::optional<ValueProducer> operation;

  For(parser::Token token);

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
