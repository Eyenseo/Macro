#ifndef cad_macro_ast_loop_For_h
#define cad_macro_ast_loop_For_h

#include "cad/macro/ast/Define.h"
#include "cad/macro/ast/Operator.h"
#include "cad/macro/ast/ValueProducer.h"
#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/callable/Callable.h"
#include "cad/macro/ast/loop/While.h"

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
  ::core::optional<Define> define;
  ::core::optional<ValueProducer> variable;
  ::core::optional<ValueProducer> operation;

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
