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
  enum class UserE { SOURCE, TAIL };
  enum class InternalE { STRING_END, BAD_CONVERSION, MISSING_OPERATOR };

  Parser();

  ast::Scope parse(std::string macro, std::string file_name = "Line") const;
};
}
}
}
#endif
