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
  template <typename T>
  void print_token(std::ostream& os, std::string prefix, T fun) const {
    IndentStream indent_os(os);

    indent_os << '@' << prefix << " {\n";
    indent_os.indent() << "line: " << token.line << " column: " << token.column
                       << " token: " << token.token << "\n";
    fun(indent_os);
    indent_os.dedent() << "}\n";
  }

  void print_token(std::ostream& os, std::string prefix) const {
    print_token(os, std::move(prefix), [](IndentStream&) {});
  }

public:
  parser::Token token;

  AST();
  AST(parser::Token token);

  bool operator==(const AST& other) const;
  bool operator!=(const AST& other) const;
  std::ostream& operator<<(std::ostream& os);
};
}
}
}
#endif
