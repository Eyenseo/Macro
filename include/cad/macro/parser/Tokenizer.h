#ifndef cad_macro_parser_tokenizer_h
#define cad_macro_parser_tokenizer_h

#include <string>
#include <vector>

namespace cad {
namespace macro {
namespace parser {
struct Token;
}
}
}

namespace cad {
namespace macro {
namespace parser {
namespace tokenizer {
using Macro = const std::string&;

/**
 * @brief  Tokenizes a string
 *
 * @param  macro  The macro
 *
 * @return vector of Token instances the parser::parse method will consume
 */
std::vector<Token> tokenize(Macro macro);
}
}
}
}
#endif
