#ifndef cad_macro_parser_Analyser_h
#define cad_macro_parser_Analyser_h

#include <string>
#include <memory>
#include <vector>

namespace cad {
namespace macro {
namespace ast {
class Scope;
}
namespace parser {
class Message;
}
}
}

namespace cad {
namespace macro {
namespace parser {
std::shared_ptr<std::vector<std::vector<Message>>>
analyse(const ast::Scope& scope, std::string file = "Anonymous");
}
}
}
#endif
