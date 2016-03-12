#ifndef cad_macro_parser_Token_h
#define cad_macro_parser_Token_h

#include <string>

namespace cad {
namespace macro {
namespace parser {
struct Token {
  size_t line;
  size_t column;
  std::string token;

  Token();
  Token(size_t line, size_t column, std::string token);
};
}
}
}
#endif
