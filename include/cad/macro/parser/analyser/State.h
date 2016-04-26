#ifndef cad_macro_parser_analyser_State_h
#define cad_macro_parser_analyser_State_h

#include "cad/macro/parser/Message.h"
#include "cad/macro/parser/analyser/Stack.h"

namespace cad {
namespace macro {
namespace ast {
class Scope;
}
}
}

namespace cad {
namespace macro {
namespace parser {
namespace analyser {
struct State {
  using MessageStack = std::vector<Message>;

public:
  Stack stack;
  std::reference_wrapper<const ast::Scope> scope;
  bool loop;
  bool root_scope;

public:
  State(const ast::Scope& s);
  State(State& parent, const ast::Scope& s, bool l = false);
};
}
}
}
}
#endif
