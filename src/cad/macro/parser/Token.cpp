#include "cad/macro/parser/Token.h"

#include <ostream>

namespace cad {
namespace macro {
namespace parser {
Token::Token() {
}

Token::Token(size_t l, size_t c, std::string t)
    : line(l)
    , column(c)
    , token(std::move(t)) {
}

bool operator!=(const Token& first, const std::string& second) {
  return !(first == second);
}
bool operator==(const Token& first, const std::string& second) {
  return first.token == second;
}

bool operator!=(const Token& first, const char* const second) {
  return !(first == second);
}
bool operator==(const Token& first, const char* const second) {
  return first.token == second;
}

bool operator!=(const Token& first, const Token& second) {
  return !(first == second);
}
bool operator==(const Token& first, const Token& second) {
  if(&first == &second) {
    return true;
  } else {
    return first.line == second.line && first.column == second.column &&
           first.token == second.token;
  }
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
  os << "Token{line: " << token.line << " column: " << token.column
     << " token: " << token.token << "}";
  return os;
}
}
}
}
