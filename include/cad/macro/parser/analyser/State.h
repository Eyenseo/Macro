#ifndef cad_macro_parser_analyser_State_h
#define cad_macro_parser_analyser_State_h

#include "cad/macro/parser/Message.h"
#include "cad/macro/parser/analyser/Stack.h"

namespace cad {
namespace macro {
namespace ast {
struct Scope;
}
}
}

namespace cad {
namespace macro {
namespace parser {
namespace analyser {
/**
 * @brief  The State struct is similar to the one used by the Interpreter it
 *         tracks all information that is additionally needed to check the ast.
 */
struct State {
  using MessageStack = std::vector<Message>;

public:
  Stack stack;
  std::reference_wrapper<const ast::Scope> scope;
  bool loop;
  bool root_scope;

public:
  /**
   * @brief  Ctor
   *
   * @param  s     ast::Scope that is currently analysed
   */
  State(const ast::Scope& s);
  /**
   * @brief  Ctor
   *
   * @param  parent  The parent state from the containing ast::Scope
   * @param  scope   ast::Scope to be analysed
   * @param  loop    true if it is a loop scope else false
   */
  State(State& parent, const ast::Scope& scope, bool loop = false);
};
}
}
}
}
#endif
