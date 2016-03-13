#ifndef cad_macro_ast_Define_h
#define cad_macro_ast_Define_h

#include "cad/macro/ast/AST.h"
#include "cad/macro/ast/executable/Function.h"
#include "cad/macro/ast/executable/EntryFunction.h"

#include "cad/macro/IndentStream.h"

#include <core/variant.hpp>
#include <core/optional.hpp>

namespace cad {
namespace macro {
namespace ast {
class Define : public AST {
  using Function = executable::Function;
  using EntryFunction = executable::EntryFunction;
  using Definition = core::variant<Function, EntryFunction>;
  template <typename T>
  using valid_type =
      typename std::enable_if<std::is_base_of<AST, T>::value &&
                                  (std::is_same<Function, T>::value ||
                                   std::is_same<EntryFunction, T>::value),
                              bool>::type;
  core::optional<Definition> def_;

public:
  Define(parser::Token token);

  template <typename T, valid_type<T> = true>
  void define(T&& ele) {
    def_ = Definition(std::forward<T>(ele));
  }

  friend std::ostream& operator<<(std::ostream& os, const Define& ast) {
    ast.print_token(os, "Define", [&ast](IndentStream& os) {
      if(ast.def_) {
        // TODO check if this works https://llvm.org/bugs/show_bug.cgi?id=26929
        // ast.def_->match([&os](const Function& v) { os << v; },
        //                 [&os](const EntryFunction& v) { os << v; });
        auto d = *ast.def_;

        switch(d.which()) {
        case 0:
          os << core::get<0>(d);
          break;
        case 1:
          os << core::get<1>(d);
          break;
        }
      }
    });

    return os;
  }
};
}
}
}
#endif
