#ifndef cad_macro_interpreter_Interpreter_h
#define cad_macro_interpreter_Interpreter_h

#include <core/any.hpp>

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
class BinaryOperator;
class Break;
class Define;
class Define;
class Return;
class Scope;
class Scope;
class Scope;
class UnaryOperator;
class ValueProducer;
namespace callable {
class Callable;
class Function;
}
namespace logic {
class If;
}
namespace loop {
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

  //////////////////////////////////////////
  /// Helper
  //////////////////////////////////////////
  bool any_to_bool(const ::core::any& any) const;

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
  ::core::any interpret_divide(State& state,
                               const ast::BinaryOperator& op) const;
  ::core::any interpret_multiply(State& state,
                                 const ast::BinaryOperator& op) const;
  ::core::any interpret_modulo(State& state,
                               const ast::BinaryOperator& op) const;
  ::core::any interpret_add(State& state, const ast::BinaryOperator& op) const;
  ::core::any interpret_subtract(State& state,
                                 const ast::BinaryOperator& op) const;
  ::core::any interpret_smaller(State& state,
                                const ast::BinaryOperator& op) const;
  ::core::any interpret_smaller_equal(State& state,
                                      const ast::BinaryOperator& op) const;
  ::core::any interpret_greater(State& state,
                                const ast::BinaryOperator& op) const;
  ::core::any interpret_greater_equal(State& state,
                                      const ast::BinaryOperator& op) const;
  ::core::any interpret_equal(State& state,
                              const ast::BinaryOperator& op) const;
  ::core::any interpret_not_equal(State& state,
                                  const ast::BinaryOperator& op) const;
  ::core::any interpret_and(State& state, const ast::BinaryOperator& op) const;
  ::core::any interpret_or(State& state, const ast::BinaryOperator& op) const;
  ::core::any interpret_assignment(State& state,
                                   const ast::BinaryOperator& op) const;
  ::core::any interpret_not(State& state, const ast::UnaryOperator& op) const;

  ::core::any interpret(State& state, const ast::BinaryOperator& op) const;
  ::core::any interpret(State& state, const ast::UnaryOperator& op) const;

  //////////////////////////////////////////
  /// interpret fundamentals
  //////////////////////////////////////////
  void interpret(State& state, const ast::Break& br) const;
  ::core::any interpret(State& state, const ast::logic::If& iff) const;
  ::core::any interpret(State& state, const ast::loop::DoWhile& whi) const;
  ::core::any interpret(State& state, const ast::loop::For& fo) const;
  ::core::any interpret(State& state, const ast::loop::While& whi) const;
  ::core::any interpret(State& state, const ast::Return& ret) const;
  ::core::any interpret(State& state, const ast::Scope& scope) const;
  ::core::any interpret_shared(State& state, const ast::Scope& scope) const;
  SmartRef<::core::any>
  interpret(State& state, const ast::ValueProducer& value_producer) const;

  //////////////////////////////////////////
  // interpret function
  //////////////////////////////////////////
  void add_parameter(State& state, State& outer_state,
                     const ast::ValueProducer& val,
                     const std::string& parameter) const;
  void add_parameter(State& state, State& outer_state,
                     const ast::callable::Callable& call,
                     const ast::callable::Function& fun) const;
  void add_arguments(State& state, Arguments& args,
                     const ast::callable::Function& fun) const;

  ::core::any interpret(State& state,
                        const ast::callable::Callable& call) const;
  ::core::any interpret_main(State& state, Arguments args) const;


public:
  enum class E { TODO };

  Interpreter(std::shared_ptr<CommandProvider> command_provider,
              std::shared_ptr<OperatorProvider> operator_provider);

  ::core::any interpret(std::string macro, Arguments args,
                        std::string file_name = "Line") const;
};
}
}
}
#endif
