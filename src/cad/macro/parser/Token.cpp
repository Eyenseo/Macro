#include "cad/macro/parser/Token.h"

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
}
}
}
