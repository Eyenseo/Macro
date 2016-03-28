#include "cad/macro/parser/Token.h"

#include <ostream>

namespace cad {
namespace macro {
namespace parser {
Token::Token()
    : line(0)
    , column(0)
    , token("") {
}

Token::Token(size_t l, size_t c, std::string t, std::shared_ptr<std::string> sl)
    : line(l)
    , column(c)
    , token(std::move(t))
    , source_line(std::move(sl)) {
}

bool Token::operator==(const Token& other) const {
  if(this == &other) {
    return true;
  } else {
    if(source_line && other.source_line) {
      return line == other.line && column == other.column &&
             token == other.token && *source_line == *other.source_line;
    } else if(!source_line && !other.source_line) {
      return line == other.line && column == other.column &&
             token == other.token;
    }
  }
  return false;
}

bool Token::operator!=(const Token& other) const {
  return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
  os << "@Token{line: " << token.line << " column: " << token.column
     << " token: " << token.token;
  if(token.source_line) {
    os << "line: " << *token.source_line;
  }
  os << "}\n";

  return os;
}
}
}
}
