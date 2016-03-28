#ifndef cad_macro_ast_Define_h
#define cad_macro_ast_Define_h

#include "cad/macro/ast/AST.h"
#include "cad/macro/ast/Variable.h"
#include "cad/macro/ast/callable/Function.h"
#include "cad/macro/ast/callable/EntryFunction.h"

#include "cad/macro/IndentStream.h"

#include <core/variant.hpp>
#include <core/optional.hpp>

namespace cad {
namespace macro {
namespace ast {
class Define : public AST {
  using Function = callable::Function;
  using EntryFunction = callable::EntryFunction;
  using Definition = core::variant<Function, EntryFunction, Variable>;

  void print_internals(IndentStream& os) const;

public:
  core::optional<Definition> definition;

  Define() = default;
  Define(parser::Token token);

  bool operator==(const Define& other) const;
  bool operator!=(const Define& other) const;

  friend std::ostream& operator<<(std::ostream& os, const Define& ast) {
    ast.print_token(os, "Define",
                    [&ast](IndentStream& os) { ast.print_internals(os); });

    return os;
  }
};
}
}
}
#endif
