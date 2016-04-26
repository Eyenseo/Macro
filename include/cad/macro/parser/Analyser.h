#ifndef cad_macro_parser_Analyser_h
#define cad_macro_parser_Analyser_h

#include "cad/macro/ast/Literal.h"

#include <p3/common/signal/Signal.h>

#include <string>
#include <memory>
#include <vector>

namespace cad {
namespace macro {
namespace ast {
class Define;
class Define;
class Operator;
class Scope;
class Scope;
class Scope;
class ValueProducer;
class Variable;
namespace callable {
class Callable;
class Function;
class Return;
class EntryFunction;
}
namespace logic {
class If;
}
namespace loop {
class Break;
class Continue;
class DoWhile;
class For;
class While;
}
}
namespace parser {
class Message;
namespace analyser {
struct State;
}
}
}
}

namespace cad {
namespace macro {
namespace parser {
class Analyser {
  template <typename T>
  using Signal = p3::common::signal::Signal<T>;
  using MessageStack = std::vector<Message>;
  using State = analyser::State;

public:
  enum class SignalType { START, END };

  // TODO store as static
  Signal<void(Analyser&, SignalType, const State&, const ast::Operator&)> biop;
  Signal<void(Analyser&, SignalType, const State&, const ast::loop::Break&)> br;
  Signal<void(Analyser&, SignalType, const State&, const ast::loop::Continue&)>
      con;
  Signal<void(Analyser&, SignalType, const State&,
              const ast::callable::Callable&)> call;
  Signal<void(Analyser&, SignalType, const State&,
              const ast::callable::EntryFunction&)> enfun;
  Signal<void(Analyser&, SignalType, const State&,
              const ast::callable::Function&)> fun;
  Signal<void(Analyser&, SignalType, const State&, const ast::Define&)> def;
  Signal<void(Analyser&, SignalType, const State&,
              const ast::Literal<ast::Literals::BOOL>&)> bo;
  Signal<void(Analyser&, SignalType, const State&,
              const ast::Literal<ast::Literals::DOUBLE>&)> dou;
  Signal<void(Analyser&, SignalType, const State&,
              const ast::Literal<ast::Literals::INT>&)> intt;
  Signal<void(Analyser&, SignalType, const State&,
              const ast::Literal<ast::Literals::STRING>&)> str;
  Signal<void(Analyser&, SignalType, const State&, const ast::logic::If&)> iff;
  Signal<void(Analyser&, SignalType, const State&, const ast::loop::DoWhile&)>
      dowhile;
  Signal<void(Analyser&, SignalType, const State&, const ast::loop::For&)> forr;
  Signal<void(Analyser&, SignalType, const State&, const ast::loop::While&)>
      whi;
  Signal<void(Analyser&, SignalType, const State&,
              const ast::callable::Return&)> ret;
  Signal<void(Analyser&, SignalType, const State&, const ast::Scope&)> sco;
  Signal<void(Analyser&, SignalType, const State&, const ast::Variable&)> var;

private:
  MessageStack current_message_;
  std::vector<MessageStack> messages_;
  std::string file_;

  void analyse(State& state, const ast::Operator& e);
  void analyse(State& state, const ast::loop::Break& e);
  void analyse(State& state, const ast::loop::Continue& e);
  void analyse(State& state, const ast::callable::Callable& e);
  void analyse(State& state, const ast::callable::EntryFunction& e);
  void analyse(State& state, const ast::callable::Function& e);
  void analyse(State& state, const ast::Define& e);
  void analyse(State& state, const ast::Literal<ast::Literals::BOOL>& e);
  void analyse(State& state, const ast::Literal<ast::Literals::DOUBLE>& e);
  void analyse(State& state, const ast::Literal<ast::Literals::INT>& e);
  void analyse(State& state, const ast::Literal<ast::Literals::STRING>& e);
  void analyse(State& state, const ast::logic::If& e);
  void analyse(State& state, const ast::loop::DoWhile& e);
  void analyse(State& state, const ast::loop::For& e);
  void analyse(State& state, const ast::loop::While& e);
  void analyse(State& state, const ast::callable::Return& e);
  void analyse(State& state, const ast::Scope& e);
  void analyse(State& state, const ast::Variable& e);
  void analyse(State& state, const ast::ValueProducer& e);

  void break_last_node();
  void continue_last_node();
  void return_last_node();
  void no_break_in_non_loop();
  void no_continue_in_non_loop();
  void no_return_in_root();
  void unique_callable_parameter();
  void unique_function_parameter();
  void unique_main_parameter();
  void unique_main();
  void main_in_root();
  void variable_available();
  void no_double_def_variable();
  void no_double_def_function();
  void op_operands();
  void op_operator();
  void op_assign_var();
  void function_scope();
  void main_scope();
  void if_scope();
  void do_while_scope();
  void while_scope();
  void if_con();
  void do_while_con();
  void while_con();

public:
  Analyser(std::string file = "Anonymous");

  std::vector<std::vector<Message>> analyse(const ast::Scope& scope);
};
}
}
}
#endif
