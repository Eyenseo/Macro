#ifndef cad_macro_parser_Parser_h
#define cad_macro_parser_Parser_h

#include <string>

namespace cad {
namespace macro {
namespace ast {
struct Scope;
}
}
}

namespace cad {
namespace macro {
namespace parser {
enum class UserE { SOURCE, TAIL };
enum class InternalE { BAD_CONVERSION, MISSING_OPERATOR };

/**
 * @brief  Parses the given macro
 *
 * @param  macro                   The macro
 * @param  file_name               The file name / macro name
 *
 * @return ast that can be consumed by the Interpreter
 *
 * @throws Exc<parser::UserE,      parser::UserE::SOURCE>
 * @throws Exc<parser::UserE,      parser::UserE::TAIL>
 * @throws Exc<parser::InternalE,  parser::InternalE::BAD_CONVERSION>
 * @throws Exc<parser::InternalE,  parser::InternalE::MISSING_OPERATOR>
 */
ast::Scope parse(std::string macro, std::string file_name = "Anonymous");
}
}
}
#endif
