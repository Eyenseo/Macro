#ifndef cad_macro_parser_Parser_h
#define cad_macro_parser_Parser_h

#include <string>

namespace cad {
namespace macro {
namespace ast {
class Scope;
}
}
}

namespace cad {
namespace macro {
namespace parser {
class Parser {
public:
  Parser();

  ast::Scope parse(std::string macro) const;
};
}
}
}
#endif
