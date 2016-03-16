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

Token::Token(size_t l, size_t c, std::string t)
    : line(l)
    , column(c)
    , token(std::move(t)) {
}

bool Token::operator==(const Token& other) const {
  if(this == &other) {
    return true;
  } else {
    return line == other.line && column == other.column && token == other.token;
  }
}

bool Token::operator!=(const Token& other) const {
  return !(*this == other);
}

std::ostream& Token::operator<<(std::ostream& os) {
  os << "Token{line: " << line << " column: " << column << " token: " << token
     << "}";
  return os;
}
}
}
}
