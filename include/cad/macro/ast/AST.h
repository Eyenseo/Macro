#ifndef cad_macro_ast_AST_h
#define cad_macro_ast_AST_h

#include "cad/macro/parser/Token.h"
#include "cad/macro/IndentStream.h"

#include <ostream>

namespace cad {
namespace macro {
namespace ast {
class AST {
protected:
  const parser::Token token_;

  template <typename T>
  void print_token(std::ostream& os, std::string prefix, T fun) const {
    IndentStream indent_os(os);

    indent_os << prefix << " {\n";
    indent_os.indent() << "line: " << token_.line
                       << " column: " << token_.column
                       << " token: " << token_.token << "\n";
    fun(indent_os);
    indent_os.dedent() << "}\n";
  }

  void print_token(std::ostream& os, std::string prefix) const {
    IndentStream indent_os(os);

    indent_os << prefix << " {\n";
    indent_os.indent() << "line: " << token_.line
                       << " column: " << token_.column
                       << " token: " << token_.token << "\n";
    indent_os.dedent() << "}\n";
  }

public:
  AST(parser::Token token);
  virtual ~AST() = default;

  friend bool operator==(const AST& first, const AST& second) {
    if(&first == &second) {
      return true;
    } else {
      return first.token_ == second.token_;
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const AST& ast) {
    ast.print_token(os, "AST");
    return os;
  }
};
}
}
}
#endif
