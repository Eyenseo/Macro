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

std::vector<Token> tokenize(Macro macro);
}
}
}
}
#endif
