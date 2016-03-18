#ifndef cad_macro_ast_loop_DoWhile_h
#define cad_macro_ast_loop_DoWhile_h

#include "cad/macro/ast/loop/While.h"

namespace cad {
namespace macro {
namespace ast {
namespace loop {
class DoWhile : public While {
public:
  DoWhile() = default;
  DoWhile(parser::Token token);

  bool operator==(const DoWhile& other) const;
  bool operator!=(const DoWhile& other) const;

  friend std::ostream& operator<<(std::ostream& os, const DoWhile& ast) {
    ast.print_token(os, "DoWhile",
                    [&ast](IndentStream& os) { ast.print_internals(os); });
    return os;
  }
};
}
}
}
}
#endif
