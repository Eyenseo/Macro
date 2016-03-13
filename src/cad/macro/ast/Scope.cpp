#include "cad/macro/ast/Scope.h"

#include "cad/macro/IndentStream.h"

namespace cad {
namespace macro {
namespace ast {
Scope::Scope(parser::Token token)
    : AST(std::move(token)) {
}


void Scope::print_scope(std::ostream& os, const Scope& scope) {
  IndentStream indent_os(os);

  indent_os << "Scope {\n";
  indent_os.indent() << "line: " << scope.token_.line
                     << " column: " << scope.token_.column
                     << " token: " << scope.token_.token << "\n";
  for(auto& v : scope.nodes_) {
    print_var(indent_os, v);
  }
  indent_os.dedent() << "}\n";
}

void Scope::print_var(IndentStream& os, const Var& var) {
  var.match([&os](const Define& v) { os << v; },
            [&os](const Function& v) { os << v; },
            [&os](const EntryFunction& v) { os << v; },
            [&os](const Scope& v) { print_scope(os, v); });
}


bool operator==(const Scope& first, const AST& ast) {
  auto second = dynamic_cast<const Scope*>(&ast);
  if(second) {
    return first == *second;
  }
  return false;
}
}
}
}
