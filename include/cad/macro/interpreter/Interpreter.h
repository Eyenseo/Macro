#ifndef cad_macro_interpreter_Interpreter_h
#define cad_macro_interpreter_Interpreter_h

#include <any.hpp>

#include <iostream>
#include <memory>

namespace cad {
namespace core {
namespace command {
class CommandProvider;
namespace argument {
class Arguments;
}
}
}
namespace macro {
namespace ast {
class Define;
class Define;
class Operator;
class Scope;
class Scope;
class Scope;
class ValueProducer;
namespace callable {
class Callable;
class Function;
class Return;
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
namespace interpreter {
class Stack;
class OperatorProvider;
}
}
}

namespace cad {
namespace macro {
namespace interpreter {
class Interpreter {
protected:
  using CommandProvider = cad::core::command::CommandProvider;
  using Arguments = cad::core::command::argument::Arguments;

  struct State;
  template <typename T>
  struct SmartRef;

  std::shared_ptr<CommandProvider> command_provider_;
  std::shared_ptr<OperatorProvider> operator_provider_;
  std::reference_wrapper<std::ostream> out_;

  //////////////////////////////////////////
  /// Helper
  //////////////////////////////////////////
  bool any_to_bool(const linb::any& any) const;

  //////////////////////////////////////////
  /// define
  //////////////////////////////////////////
  void define_variable(State& state, const ast::Define& def) const;
  void define_function(State& state, const ast::Define& def) const;
  void define_functions(State& state, const ast::Scope& scope) const;

  //////////////////////////////////////////
  /// Interpret operator
  //////////////////////////////////////////
  [[noreturn]] void interpret_none() const;
  // Binary helper
  linb::any interpret_divide(State& state, const ast::Operator& op) const;
  linb::any interpret_multiply(State& state, const ast::Operator& op) const;
  linb::any interpret_modulo(State& state, const ast::Operator& op) const;
  linb::any interpret_add(State& state, const ast::Operator& op) const;
  linb::any interpret_subtract(State& state, const ast::Operator& op) const;
  linb::any interpret_smaller(State& state, const ast::Operator& op) const;
  linb::any interpret_smaller_equal(State& state,
                                    const ast::Operator& op) const;
  linb::any interpret_greater(State& state, const ast::Operator& op) const;
  linb::any interpret_greater_equal(State& state,
                                    const ast::Operator& op) const;
  linb::any interpret_equal(State& state, const ast::Operator& op) const;
  linb::any interpret_not_equal(State& state, const ast::Operator& op) const;
  linb::any interpret_and(State& state, const ast::Operator& op) const;
  linb::any interpret_or(State& state, const ast::Operator& op) const;
  linb::any interpret_assignment(State& state, const ast::Operator& op) const;
  // Unary helper
  linb::any interpret_not(State& state, const ast::Operator& op) const;
  linb::any interpret_typeof(State& state, const ast::Operator& op) const;
  linb::any interpret_print(State& state, const ast::Operator& op) const;
  linb::any interpret_negative(State& state, const ast::Operator& op) const;
  linb::any interpret_positive(State& state, const ast::Operator& op) const;

  linb::any interpret(State& state, const ast::Operator& op) const;

  //////////////////////////////////////////
  /// interpret fundamentals
  //////////////////////////////////////////
  void interpret(State& state, const ast::loop::Break& br) const;
  void interpret(State& state, const ast::loop::Continue& br) const;
  linb::any interpret(State& state, const ast::logic::If& iff) const;
  linb::any interpret(State& state, const ast::loop::DoWhile& whi) const;
  linb::any interpret(State& state, const ast::loop::For& fo) const;
  linb::any interpret(State& state, const ast::loop::While& whi) const;
  linb::any interpret(State& state, const ast::callable::Return& ret) const;
  linb::any interpret(State& state, const ast::Scope& scope) const;
  linb::any interpret_shared(State& state, const ast::Scope& scope) const;
  SmartRef<linb::any> interpret(State& state,
                                const ast::ValueProducer& value_producer) const;

  //////////////////////////////////////////
  // interpret function
  //////////////////////////////////////////
  Arguments args_from_call(State& state, const ast::callable::Callable& call,
                           const Arguments& command_args) const;

  void add_parameter(State& state, State& outer_state,
                     const ast::ValueProducer& val,
                     const std::string& parameter) const;
  void add_parameter(State& state, State& outer_state,
                     const ast::callable::Callable& call,
                     const ast::callable::Function& fun) const;
  void add_arguments(State& state, Arguments& args,
                     const ast::callable::Function& fun) const;

  linb::any interpret(State& state, const ast::callable::Callable& call) const;
  linb::any interpret_main(State& state, Arguments args) const;

public:
  enum class E { BAD_BOOL_CAST, MISSING_FUNCTION, TAIL };

  Interpreter(std::shared_ptr<CommandProvider> command_provider,
              std::shared_ptr<OperatorProvider> operator_provider,
              std::ostream& out = std::cout);

  linb::any interpret(std::string macro, Arguments args, std::string scope = "",
                      std::string file_name = "Anonymous") const;
};
}
}
}
#endif
