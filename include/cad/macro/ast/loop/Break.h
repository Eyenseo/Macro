#ifndef cad_macro_ast_loop_Break_h
#define cad_macro_ast_loop_Break_h

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
class Break : public AST {
public:
  Break();
  Break(parser::Token token);

  friend std::ostream& operator<<(std::ostream& os, const Break& ast) {
    ast.print_token(os, "Break");
    return os;
  }
};
}
}
}
}
#endif
