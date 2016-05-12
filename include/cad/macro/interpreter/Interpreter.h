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
struct Define;
struct Operator;
struct Scope;
struct ValueProducer;
namespace callable {
struct Callable;
struct Function;
struct Return;
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
namespace interpreter {
class Stack;
class OperatorProvider;
}
}
}

namespace cad {
namespace macro {
namespace interpreter {
/**
 * @brief  The Interpreter accepts a macro as std::string, converts it with the
 *         parser::parse method and executes the obtained abstract syntax tree
 */
class Interpreter {
protected:
  using CommandProvider = cad::core::command::CommandProvider;
  using Arguments = cad::core::command::argument::Arguments;

  /**
   * @brief   The State struct will hold the information of the interpretation.
   * @details The information is ast::Scope local and new instances of this
   *          struct will be constructed if a new ast::Scope is interpreted
   */
  struct State;
  /**
   * @brief   The SmartFref struct is a helper type that holds a value and an
   *          reference to __a__ value.
   * @details The struct holds a value and a std::reference_wrapper to said
   *          value. The reference_wrapper can be used to set an reference to a
   *          value that is not the value of this struct. This guarantees that
   *          the reference is always valid and points to the desired value.
   *
   */
  struct SmartRef;

  std::shared_ptr<CommandProvider> command_provider_;
  std::shared_ptr<OperatorProvider> operator_provider_;
  std::reference_wrapper<std::ostream> out_;

  //////////////////////////////////////////
  /// Helper
  //////////////////////////////////////////
  /**
   * @brief   Converts the given any instance to a boolean if possible
   * @details The function checks if the value of the any instance is a boolean
   *          value, if not it tries to convert the value with the help of the
   *          OperatorProvider to a boolean via the boolean operator.
   *
   * @param   any   The any instance to be converted to a boolean
   *
   * @return  true if the converted any value evaluates to true, false otherwise
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   */
  bool any_to_bool(const linb::any& any) const;

  /**
   * @brief  Creates an Arguments instance that holds the copies of the values
   *         the ast::Variables represent
   *
   * @param  state         The state of the interpretation
   * @param  call          The ast::callable::Callable instance that represents
   *                       a core::Command
   * @param  command_args  The Arguments the core::Command can use
   *
   * @return Arguments instance that can be passed to a core::Command that
   *         realises the callable
   */
  Arguments args_from_call(State& state, const ast::callable::Callable& call,
                           const Arguments& command_args) const;
  /**
   * @brief  Adds the ast::Variable parameter from the calling (outer)
   *         ast::Scope to the new (inner) ast::Scope
   *
   * @param  state        The state of the interpretation
   * @param  outer_state  The outer state of the interpretation
   * @param  val          The ast::ValueProducer that provides the ast::Function
   *                      with the value of the ast::Variable
   * @param  parameter    The name of the ast::Function parameter
   */
  void add_parameter(State& state, State& outer_state,
                     const ast::ValueProducer& val,
                     const std::string& parameter) const;
  /**
   * @brief  Adds the ast::Variable parameter from the calling (outer)
   *         ast::Scope to the new (inner) ast::Scope
   *
   * @param  state        The state of the interpretation
   * @param  outer_state  The outer state of the interpretation
   * @param  call         The ast::callable::Callable instance that has the
   *                      parameter the ast::callable::Function is called with
   * @param  fun          The ast::callable::Function that will be called /
   *                      interpreted
   */
  void add_parameter(State& state, State& outer_state,
                     const ast::callable::Callable& call,
                     const ast::callable::Function& fun) const;

  /**
   * @brief  Adds the give Arguments from the Interpreter to the root ast::Scope
   *
   * @param  state  The state of the interpretation
   * @param  args   The Arguments given from the Interpreter
   * @param  fun    The ast::callable::Function instance that will be
   * interpreted
   */
  void add_arguments(State& state, Arguments& args,
                     const ast::callable::Function& fun) const;

  //////////////////////////////////////////
  /// define
  //////////////////////////////////////////
  /**
   * @brief   Defines a ast::Variable
   * @details If the given ast::Define holds a ast::Variable it will be defined
   *          after this function call. If the ast::Define does not hold a
   *          ast::Variable nothing happens.
   *
   * @param   state  The state of the interpretation
   * @param   def    The ast::Define object which possibly holds a ast::Variable
   */
  void define_variable(State& state, const ast::Define& def) const;
  /**
   * @brief   Defines a ast::Function or ast::EntryFunction
   * @details If the given ast::Define holds a ast::Function or
   *          ast::EntryFunction it will be defined after this function call. If
   *          the ast::Define does not hold a ast::Function or
   *          ast::EntryFunction nothing happens.
   *
   * @param   state  The state of the interpretation
   * @param   def    The ast::Define object which possibly holds a ast::Function
   *                 or ast::EntryFunction
   */
  void define_function(State& state, const ast::Define& def) const;
  /**
   * @brief  The function will define all ast::Function or ast::EntryFunction in
   *         the given Scope
   *
   * @param  state  The state of the interpretation
   * @param  scope  The scope that possible holds the definitions of
   *                ast::Function or ast::EntryFunction
   */
  void define_functions(State& state, const ast::Scope& scope) const;

  //////////////////////////////////////////
  /// Interpret operator
  //////////////////////////////////////////
  /**
   * @brief   assert(false) -- the parser::Analyser should have caught this.
   */
  [[noreturn]] void interpret_none() const;
  // Binary helper
  /**
   * @brief  Interprets the given ast::Operator instance as 'divide'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_divide(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'multiply'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_multiply(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'modulo'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_modulo(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'add'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_add(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'subtract'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_subtract(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'smaller'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_smaller(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'smaller equal'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_smaller_equal(State& state,
                                    const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'greater'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_greater(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'greater equal'
   *
   * @param  state  The state of the interpretation
   * @param  op     The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_greater_equal(State& state,
                                    const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'equal'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_equal(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'not equal'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_not_equal(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'and'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_and(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'or'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given types in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_or(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'assignment'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the ast::Variable after the assignment
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_assignment(State& state, const ast::Operator& op) const;
  // Unary helper
  /**
   * @brief  Interprets the given ast::Operator instance as 'not'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given type in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_not(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'typeof'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given type in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_typeof(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'print'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given type in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_print(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'negative'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given type in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_negative(State& state, const ast::Operator& op) const;
  /**
   * @brief  Interprets the given ast::Operator instance as 'positive'
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator specified for the given type in the
   *         OperatorProvider
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_positive(State& state, const ast::Operator& op) const;

  /**
   * @brief  Interprets the given ast::Operator
   *
   * @param  state   The state of the interpretation
   * @param  op      The ast::Operation instance to interpret
   *
   * @return result of the operator
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret(State& state, const ast::Operator& op) const;

  //////////////////////////////////////////
  /// interpret fundamentals
  //////////////////////////////////////////
  /**
   * @brief  Interprets the given ast::loop::Break instance
   *
   * @param  state   The state of the interpretation
   * @param  br      The ast::loop::Break instance to interpret
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  void interpret(State& state, const ast::loop::Break& br) const;
  /**
   * @brief  Interprets the given ast::loop::Continue instance
   *
   * @param  state   The state of the interpretation
   * @param  con     The ast::loop::Continue instance to interpret
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  void interpret(State& state, const ast::loop::Continue& con) const;
  /**
   * @brief  Interprets the given ast::logic::If instance
   *
   * @param  state   The state of the interpretation
   * @param  iff     The ast::logic::If instance to interpret
   *
   * @return result of the interpretation
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret(State& state, const ast::logic::If& iff) const;
  /**
   * @brief  Interprets the given ast::loop::DoWhile instance
   *
   * @param  state   The state of the interpretation
   * @param  whi     The ast::loop::DoWhile instance to interpret
   *
   * @return result of the interpretation
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret(State& state, const ast::loop::DoWhile& whi) const;
  /**
   * @brief  Interprets the given ast::loop::For instance
   *
   * @param  state   The state of the interpretation
   * @param  fo      The ast::loop::For instance to interpret
   *
   * @return result of the interpretation
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret(State& state, const ast::loop::For& fo) const;
  /**
   * @brief  Interprets the given ast::loop::While instance
   *
   * @param  state   The state of the interpretation
   * @param  whi     The ast::loop::While instance to interpret
   *
   * @return result of the interpretation
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret(State& state, const ast::loop::While& whi) const;
  /**
   * @brief  Interprets the given ast::callable::Return instance
   *
   * @param  state   The state of the interpretation
   * @param  ret     The ast::callable::Return instance to interpret
   *
   * @return result of the interpretation
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret(State& state, const ast::callable::Return& ret) const;
  /**
   * @brief  Interprets the given ast::callable::Callable instance
   *
   * @param  state   The state of the interpretation
   * @param  call    The ast::callable::Callable instance to interpret
   *
   * @return result of the interpretation
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret(State& state, const ast::callable::Callable& call) const;
  /**
   * @brief  Interprets the given ast::Scope instance
   *
   * @param  state   The state of the interpretation
   * @param  scope   The ast::Scope instance to interpret
   *
   * @return result of the interpretation
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret(State& state, const ast::Scope& scope) const;

  /**
   * @brief  Interprets the given ast::ValueProducer instance
   *
   * @param  state           The state of the interpretation
   * @param  value_producer  The ast::ValueProducer instance to interpret
   *
   * @return result of the interpretation
   *
   * @throws Exc<E,          E::BAD_BOOL_CAST>
   * @throws Exc<E,          E::MISSING_FUNCTION>
   * @throws Exc<E,          E::TAIL>
   */
  SmartRef interpret(State& state,
                     const ast::ValueProducer& value_producer) const;
  /**
   * @brief  Interprets the given ast::Scope instance without creating a new
   *         Stack
   *
   * @param  state   The state of the interpretation
   * @param  scope   The ast::Scope instance to interpret
   *
   * @return result of the interpretation
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_shared(State& state, const ast::Scope& scope) const;

  //////////////////////////////////////////
  // interpret function
  //////////////////////////////////////////
  /**
   * @brief  Interprets the main function of the macro
   *
   * @param  state   The state of the interpretation
   * @param  args    The Arguments to interpret the main function with
   *
   * @return result of the interpretation
   *
   * @throws Exc<E,  E::BAD_BOOL_CAST>
   * @throws Exc<E,  E::MISSING_FUNCTION>
   * @throws Exc<E,  E::TAIL>
   */
  linb::any interpret_main(State& state, Arguments args) const;

public:
  enum class E { BAD_BOOL_CAST, MISSING_FUNCTION, TAIL };

  /**
   * @brief  Ctor
   *
   * @param  command_provider   The CommandProvider to get core::Command
   *                            instances from
   * @param  operator_provider  The OperatorProvider to eval ast::Operator
   *                            instances with
   * @param  out                The output stream
   */
  Interpreter(std::shared_ptr<CommandProvider> command_provider,
              std::shared_ptr<OperatorProvider> operator_provider,
              std::ostream& out = std::cout);

  /**
   * @brief  Interprets a given macro
   *
   * @param  macro                   The macro to interpret
   * @param  args                    The arguments to interpret the macro with
   * @param  scope                   The scope from which the interpretation was
   *                                 started in, to get the right core::Command
   *                                 instances
   * @param  file_name               The file name / name of the macro.
   *
   * @return result of the interpretation
   *
   * @throws Exc<E,                  E::BAD_BOOL_CAST>
   * @throws Exc<E,                  E::MISSING_FUNCTION>
   * @throws Exc<E,                  E::TAIL>
   * @throws Exc<parser::UserE,      parser::UserE::SOURCE>
   * @throws Exc<parser::UserE,      parser::UserE::TAIL>
   * @throws Exc<parser::InternalE,  parser::InternalE::BAD_CONVERSION>
   * @throws Exc<parser::InternalE,  parser::InternalE::MISSING_OPERATOR>
   */
  linb::any interpret(std::string macro, Arguments args, std::string scope = "",
                      std::string file_name = "Anonymous") const;
};
}
}
}
#endif
