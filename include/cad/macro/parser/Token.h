#ifndef cad_macro_parser_Token_h
#define cad_macro_parser_Token_h

#include <memory>
#include <string>

namespace cad {
namespace macro {
namespace parser {
struct Token {
  size_t line;
  size_t column;
  std::string token;
  std::shared_ptr<std::string> source_line;

  Token();
  Token(size_t line, size_t column, std::string token,
        std::shared_ptr<std::string> source_line = nullptr);

  bool operator==(const Token& other) const;
  bool operator!=(const Token& other) const;
};
std::ostream& operator<<(std::ostream& os, const Token& token);
}
}
}
#endif
