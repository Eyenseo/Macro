#include "cad/macro/parser/analyser/State.h"

namespace cad {
namespace macro {
namespace parser {
namespace analyser {
State::State(const ast::Scope& s)
    : stack()
    , scope(s)
    , loop(false)
    , root_scope(false) {
}
State::State(State& parent, const ast::Scope& s, bool l)
    : stack()
    , scope(s)
    , loop(l ? l : parent.loop)
    , root_scope(parent.root_scope) {
  stack.parent = &parent.stack;
}
}
}
}
}
