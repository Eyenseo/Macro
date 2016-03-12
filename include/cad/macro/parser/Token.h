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
bool operator==(const Token& first, const Token& second);
std::ostream& operator<<(std::ostream& os, const Token& token);
}
}
}
#endif
