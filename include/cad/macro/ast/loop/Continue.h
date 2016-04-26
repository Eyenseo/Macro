#ifndef cad_macro_ast_loop_Continue_h
#define cad_macro_ast_loop_Continue_h

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
namespace loop {
class Continue : public AST {
public:
  Continue();
  Continue(parser::Token token);

  friend std::ostream& operator<<(std::ostream& os, const Continue& ast) {
    ast.print_token(os, "Continue");
    return os;
  }
};
}
}
}
}
#endif
