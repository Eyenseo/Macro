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
struct Define;
struct Define;
struct Operator;
struct Scope;
struct Scope;
struct Scope;
struct ValueProducer;
struct Variable;
namespace callable {
struct Callable;
struct Function;
struct Return;
struct EntryFunction;
}
namespace logic {
struct If;
}
namespace loop {
struct Break;
struct Continue;
struct DoWhile;
struct For;
struct While;
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
/**
 * @brief   The Analyser is a class that validates the ast before the
 *          Interpreter interprets it.
 *
 * @details This two step process is used because the parsing should not check
 *          for things that are not syntax related but language related and the
 *          interpreter doesn't need to have this additionally runtime cost
 */
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

  //////////////////////////////////////////
  // Walker
  //////////////////////////////////////////
  /**
   * @brief  Analyses ast::Operator by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::Operator& e);
  /**
   * @brief  Analyses ast::loop::Break by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::loop::Break& e);
  /**
   * @brief  Analyses ast::loop::Continue by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::loop::Continue& e);
  /**
   * @brief  Analyses ast::callable::Callable by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::callable::Callable& e);
  /**
   * @brief  Analyses ast::callable::EntryFunction by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::callable::EntryFunction& e);
  /**
   * @brief  Analyses ast::callable::Function by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::callable::Function& e);
  /**
   * @brief  Analyses ast::Define by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::Define& e);
  /**
   * @brief  Analyses ast::Literal<ast::Literals::BOOL> by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::Literal<ast::Literals::BOOL>& e);
  /**
   * @brief  Analyses ast::Literal<ast::Literals::DOUBLE> by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::Literal<ast::Literals::DOUBLE>& e);
  /**
   * @brief  Analyses ast::Literal<ast::Literals::INT> by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::Literal<ast::Literals::INT>& e);
  /**
   * @brief  Analyses ast::Literal<ast::Literals::STRING> by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::Literal<ast::Literals::STRING>& e);
  /**
   * @brief  Analyses ast::logic::If by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::logic::If& e);
  /**
   * @brief  Analyses ast::loop::DoWhile by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::loop::DoWhile& e);
  /**
   * @brief  Analyses ast::loop::For by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::loop::For& e);
  /**
   * @brief  Analyses ast::loop::While by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::loop::While& e);
  /**
   * @brief  Analyses ast::callable::Return by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::callable::Return& e);
  /**
   * @brief  Analyses ast::Scope by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::Scope& e);
  /**
   * @brief  Analyses ast::Variable by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::Variable& e);
  /**
   * @brief  Analyses ast::ValueProducer by sending signals
   *
   * @param  state  The state
   * @param  e      The ast element to analyse
   */
  void analyse(State& state, const ast::ValueProducer& e);

  //////////////////////////////////////////
  // Tests
  //////////////////////////////////////////

  /**
   * @brief  Checks that ast::loop::Break is the last node in the ast::Scope if
   *         it is in the ast::Scope
   */
  void break_last_node();
  /**
   * @brief  Checks that ast::loop::Continue is the last node in the ast::Scope
   *         if it is in the ast::Scope
   */
  void continue_last_node();
  /**
   * @brief  Checks that ast::Return is the last node in the ast::Scope
   *         if it is in the ast::Scope
   */
  void return_last_node();
  /**
   * @brief  Checks that ast::loop::Break is only in loop ast::Scopes
   */
  void no_break_in_non_loop();
  /**
   * @brief  Checks that ast::loop::Continue is only in loop ast::Scopes
   */
  void no_continue_in_non_loop();
  /**
   * @brief  Checks that ast::Return is not in the root scope
   */
  void no_return_in_root();
  /**
   * @brief  Checks that the parameter of ast::callable::Callable are uniquely
   *         named
   */
  void unique_callable_parameter();
  /**
   * @brief  Checks that the parameter of ast::callable::Function are uniquely
   *         named
   */
  void unique_function_parameter();
  /**
   * @brief  Checks that the parameter of ast::callable::EntryFunction are
   *         uniquely named
   */
  void unique_main_parameter();
  /**
   * @brief  Checks that only one main function is defined
   */
  void unique_main();
  /**
   * @brief  checks that the defined main function is in the root ast::Scope
   */
  void main_in_root();
  /**
   * @brief  Checks that the used variable is indeed available / defined
   */
  void variable_available();
  /**
   * @brief  Checks that no variable is defined twice in the same scope
   */
  void no_double_def_variable();
  /**
   * @brief  Checks that no function is defined twice in the same scope
   */
  void no_double_def_function();
  /**
   * @brief  Checks that all ast::Operator instances have the right amount of
   *         operands (binary and unary)
   */
  void op_operands();
  /**
   * @brief  Checks that all ast::Operator instances have a operation
   */
  void op_operator();
  /**
   * @brief  Checks that the left hand side of the assignment operator is a
   *         variable
   */
  void op_assign_var();
  /**
   * @brief  Checks that the ast::callable::Function instance has a scope - not
   *         needed but better safe than sorry
   */
  void function_scope();
  /**
   * @brief  Checks that the ast::callable::EntryFunction instance has a scope -
   *         not needed but better safe than sorry
   */
  void main_scope();
  /**
   * @brief  Checks that the ast::logic::If instance has a scope - not needed
   *         but better safe than sorry
   */
  void if_scope();
  /**
   * @brief  Checks that the ast::loop::DoWhile instance has a scope - not
   *         needed but better safe than sorry
   */
  void do_while_scope();
  /**
   * @brief  Checks that the ast::loop::While instance has a scope - not needed
   *         but better safe than sorry
   */
  void while_scope();
  /**
   * @brief  Checks that the ast::logic::If instance has a condition - not
   *         needed but better safe than sorry
   */
  void if_con();
  /**
   * @brief  Checks that the ast::loop::DoWhile instance has a condition - not
   *         needed but better safe than sorry
   */
  void do_while_con();
  /**
   * @brief  Checks that the ast::loop::While instance has a condition - not
   *         needed but better safe than sorry
   */
  void while_con();

public:
  /**
   * @brief  Ctor
   *
   * @param  file  The file name / macro name
   */
  Analyser(std::string file = "Anonymous");

  /**
   * @brief  Analyses the given ast
   *
   * @return a vector of Messages that contain all errors with stack that were
   *         found
   */
  std::vector<std::vector<Message>> analyse(const ast::Scope& scope);
};
}
}
}
#endif
