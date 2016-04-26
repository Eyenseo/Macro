#include "cad/macro/interpreter/Interpreter.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/interpreter/Stack.h"
#include "cad/macro/interpreter/OperatorProvider.h"
#include "cad/macro/parser/Parser.h"

#include <cad/core/command/argument/Arguments.h>
#include <cad/core/command/CommandProvider.h>
#include <cad/core/command/CommandInvoker.h>

#include <cassert>

namespace cad {
namespace macro {
namespace interpreter {
namespace {
using namespace ast;
using namespace ast::callable;
using namespace ast::loop;
using namespace ast::logic;

template <typename FUN>
void add_exception_info(const parser::Token& token, const std::string& file,
                        Exc<Interpreter::E, Interpreter::E::TAIL>& e, FUN fun) {
  e << file << ':' << token.line << ':' << token.column << ": ";
  fun();
  if(token.source_line) {
    e << '\n' << *token.source_line << '\n'
      << std::string(token.column - 1, ' ') << "^";
  }
}
}

struct Interpreter::State {
  std::shared_ptr<Stack> stack;
  std::string scope;
  std::string file;
  bool breaking;
  bool loopscope;
  bool returning;

  State(std::string s, std::string f)
      : stack(std::make_shared<Stack>())
      , scope(std::move(s))
      , file(std::move(f))
      , breaking(false)
      , loopscope(false)
      , returning(false) {
  }
  State(const State& other, std::shared_ptr<Stack> s)
      : stack(std::move(s))
      , scope(other.scope)
      , file(other.file)
      , breaking(other.breaking)
      , loopscope(other.loopscope)
      , returning(other.returning) {
  }
};

template <typename T>
struct Interpreter::SmartRef {
  T value;

  std::reference_wrapper<T> ref;
  SmartRef()
      : ref(value) {
  }
  operator ::core::any&() {
    return ref.get();
  }
};

Interpreter::Interpreter(std::shared_ptr<CommandProvider> command_provider,
                         std::shared_ptr<OperatorProvider> operator_provider,
                         std::ostream& out)
    : command_provider_(std::move(command_provider))
    , operator_provider_(std::move(operator_provider))
    , out_(out) {
}

::core::any Interpreter::interpret(std::string macro, Arguments args,
                                   std::string command_scope,
                                   std::string file_name) const {
  auto scope = parser::parse(macro, file_name);
  State state(std::move(command_scope), file_name);

  interpret(state, scope);
  return interpret_main(state, std::move(args));
}

//////////////////////////////////////////
/// Helper
//////////////////////////////////////////
bool Interpreter::any_to_bool(const ::core::any& any) const {
  if(any.type() == typeid(bool)) {  // no need to convert
    return ::core::any_cast<bool>(any);
  } else {  // need to convert
    auto b =
        operator_provider_->eval(OperatorProvider::UnaryOperation::BOOL, any);

    if(b.type() == typeid(bool)) {
      return ::core::any_cast<bool>(b);
    } else {  // not a bool type ...
      Exc<E, E::BAD_BOOL_CAST> e(__FILE__, __LINE__, "Bad bool cast");
      e << "Tried cast '" << any.type().name()
        << "' to bool but the operator returned '" << b.type().name() << "'.";
      throw e;
    }
  }
}

//////////////////////////////////////////
/// define
//////////////////////////////////////////
void Interpreter::define_functions(State& state, const Scope& scope) const {
  for(const auto& n : scope.nodes) {
    n.match(
        [this, &state](const Define& def) { define_function(state, def); },  //
        [this](const loop::Break&) {},                                       //
        [this](const Callable&) {},                                          //
        [this](const DoWhile&) {},                                           //
        [this](const For&) {},                                               //
        [this](const If&) {},                                                //
        [this](const Literal<Literals::BOOL>&) {},                           //
        [this](const Literal<Literals::DOUBLE>&) {},                         //
        [this](const Literal<Literals::INT>&) {},                            //
        [this](const Literal<Literals::STRING>&) {},                         //
        [this](const callable::Return&) {},                                  //
        [this](const Scope&) {},                                             //
        [this](const Operator&) {},                                          //
        [this](const While&) {},                                             //
        [this](const Variable&) {}                                           //
        );                                                                   //
  }
}
void Interpreter::define_function(State& state, const Define& def) const {
  def.definition.match(
      [&](const Function& fun) { state.stack->add_function(fun); },
      [&](const EntryFunction& fun) { state.stack->add_function(fun); },
      [&](const Variable&) {});
}
void Interpreter::define_variable(State& state, const Define& def) const {
  def.definition.match(
      [&](const Variable& var) { state.stack->add_variable(var.token.token); },
      [&](const Function&) {}, [&](const EntryFunction&) {});
}

//////////////////////////////////////////
/// Interpret operator
//////////////////////////////////////////
void Interpreter::interpret_none() const {
  assert(false); /* analyser checked */
}

//////////////////////////////////////////
/// Binary
//////////////////////////////////////////
::core::any Interpreter::interpret_divide(State& state,
                                          const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::DIVIDE, lhs, rhs);
}
::core::any Interpreter::interpret_multiply(State& state,
                                            const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::MULTIPLY, lhs, rhs);
}
::core::any Interpreter::interpret_modulo(State& state,
                                          const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::MODULO, lhs, rhs);
}
::core::any Interpreter::interpret_add(State& state, const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::ADD, lhs, rhs);
}
::core::any Interpreter::interpret_subtract(State& state,
                                            const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::SUBTRACT, lhs, rhs);
}
::core::any Interpreter::interpret_smaller(State& state,
                                           const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::SMALLER, lhs, rhs);
}
::core::any Interpreter::interpret_smaller_equal(State& state,
                                                 const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::SMALLER_EQUAL, lhs, rhs);
}
::core::any Interpreter::interpret_greater(State& state,
                                           const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::GREATER, lhs, rhs);
}
::core::any Interpreter::interpret_greater_equal(State& state,
                                                 const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::GREATER_EQUAL, lhs, rhs);
}
::core::any Interpreter::interpret_equal(State& state,
                                         const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::EQUAL, lhs, rhs);
}
::core::any Interpreter::interpret_not_equal(State& state,
                                             const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::NOT_EQUAL, lhs, rhs);
}
::core::any Interpreter::interpret_and(State& state, const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::AND, lhs, rhs);
}
::core::any Interpreter::interpret_or(State& state, const Operator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::OR, lhs, rhs);
}
::core::any Interpreter::interpret_assignment(State& state,
                                              const Operator& op) const {
  ::core::any rh;
  auto ret_par = [&](State& s, const auto& v) { rh = interpret(s, v); };
  auto lit_par = [&](const auto& v) { rh = v.data; };
  auto var_par = [&](State& s, auto v) {
    if(s.stack->has_variable(v.token.token)) {
      s.stack->variable(v.token.token, [&](::core::any& var) { rh = var; });
    } else {
      assert(false); /* analyser checked */
    }
  };

  op.right_operand->value.match(
      [&](const callable::Callable& o) { ret_par(state, o); },
      [&](const Operator& o) { ret_par(state, o); },
      [&](const Variable& o) { var_par(state, o); },
      [&](const Literal<Literals::BOOL>& c) { lit_par(c); },
      [&](const Literal<Literals::INT>& c) { lit_par(c); },
      [&](const Literal<Literals::DOUBLE>& c) { lit_par(c); },
      [&](const Literal<Literals::STRING>& c) { lit_par(c); });

  op.left_operand->value.match(
      [&](const callable::Callable&) {
        assert(false); /* analyser checked */
      },
      [&](const Operator&) {
        assert(false); /* analyser checked */
      },
      [&](const Variable& o) {
        if(!state.stack->owns_variable(o.token.token)) {
          state.stack->remove_alias(o.token.token);
          state.stack->add_variable(o.token.token);
        }
        state.stack->variable(o.token.token,
                              [&](::core::any& var) { var = rh; });
      },
      [&](const Literal<Literals::BOOL>&) {
        assert(false); /* analyser checked */
      },
      [&](const Literal<Literals::INT>&) {
        assert(false); /* analyser checked */
      },
      [&](const Literal<Literals::DOUBLE>&) {
        assert(false); /* analyser checked */
      },
      [&](const Literal<Literals::STRING>&) {
        assert(false); /* analyser checked */
      });
  return rh;
}

::core::any Interpreter::interpret(State& state, const Operator& op) const {
  try {
    switch(op.operation) {
    case Operation::NONE:
      interpret_none();  // assert
    case Operation::DIVIDE:
      return interpret_divide(state, op);
    case Operation::MULTIPLY:
      return interpret_multiply(state, op);
    case Operation::MODULO:
      return interpret_modulo(state, op);
    case Operation::ADD:
      return interpret_add(state, op);
    case Operation::SUBTRACT:
      return interpret_subtract(state, op);
    case Operation::SMALLER:
      return interpret_smaller(state, op);
    case Operation::SMALLER_EQUAL:
      return interpret_smaller_equal(state, op);
    case Operation::GREATER:
      return interpret_greater(state, op);
    case Operation::GREATER_EQUAL:
      return interpret_greater_equal(state, op);
    case Operation::EQUAL:
      return interpret_equal(state, op);
    case Operation::NOT_EQUAL:
      return interpret_not_equal(state, op);
    case Operation::AND:
      return interpret_and(state, op);
    case Operation::OR:
      return interpret_or(state, op);
    case Operation::ASSIGNMENT:
      return interpret_assignment(state, op);
    case Operation::NOT:
      return interpret_not(state, op);
    case Operation::TYPEOF:
      return interpret_typeof(state, op);
    case Operation::PRINT:
      return interpret_print(state, op);
    case Operation::NEGATIVE:
      return interpret_negative(state, op);
    case Operation::POSITIVE:
      return interpret_positive(state, op);
    }
  } catch(std::exception&) {
    Exc<E, E::TAIL> e;
    add_exception_info(op.token, state.file, e, [&e, &op]() {
      e << "At the operator '" << op.token.token << "' defined here";
    });
    std::throw_with_nested(e);
  }
  assert(false && "Reached by access after free and similar");
}

//////////////////////////////////////////
/// Unary
//////////////////////////////////////////
::core::any Interpreter::interpret_not(State& state, const Operator& op) const {
  auto rhs = interpret(state, *op.right_operand);

  using UnOp = OperatorProvider::UnaryOperation;
  return operator_provider_->eval(UnOp::NOT, rhs);
}
::core::any Interpreter::interpret_typeof(State& state,
                                          const Operator& op) const {
  auto rhs = interpret(state, *op.right_operand);

  using UnOp = OperatorProvider::UnaryOperation;
  return operator_provider_->eval(UnOp::TYPEOF, rhs);
}
::core::any Interpreter::interpret_print(State& state,
                                         const Operator& op) const {
  auto rhs = interpret(state, *op.right_operand);

  using UnOp = OperatorProvider::UnaryOperation;
  auto res =
      ::core::any_cast<std::string>(operator_provider_->eval(UnOp::PRINT, rhs));
  out_.get() << res;
  return res;
}
::core::any Interpreter::interpret_negative(State& state,
                                            const Operator& op) const {
  auto rhs = interpret(state, *op.right_operand);

  using UnOp = OperatorProvider::UnaryOperation;
  return operator_provider_->eval(UnOp::NEGATIVE, rhs);
}

::core::any Interpreter::interpret_positive(State& state,
                                            const Operator& op) const {
  auto rhs = interpret(state, *op.right_operand);

  using UnOp = OperatorProvider::UnaryOperation;
  return operator_provider_->eval(UnOp::POSITIVE, rhs);
}

//////////////////////////////////////////
/// Helper
//////////////////////////////////////////
Interpreter::SmartRef<::core::any>
Interpreter::interpret(State& state, const ValueProducer& vp) const {
  SmartRef<::core::any> f;

  vp.value.match(
      [&](const callable::Callable& o) { f.value = interpret(state, o); },
      [&](const Operator& o) { f.value = interpret(state, o); },
      [&](const Variable& o) {
        if(state.stack->has_variable(o.token.token)) {
          return state.stack->variable(o.token.token,
                                       [&](::core::any& var) { f.ref = var; });
        } else {
          assert(false); /* analyser checked */
        }
      },
      [&](const Literal<Literals::BOOL>& o) { f.value = ::core::any(o.data); },
      [&](const Literal<Literals::INT>& o) { f.value = ::core::any(o.data); },
      [&](const Literal<Literals::DOUBLE>& o) {
        f.value = ::core::any(o.data);
      },
      [&](const Literal<Literals::STRING>& o) {
        f.value = ::core::any(o.data);
      });
  return f;
}

//////////////////////////////////////////
/// interpret fundamentals
//////////////////////////////////////////
void Interpreter::interpret(State& state, const ast::loop::Break&) const {
  if(state.loopscope) {
    state.breaking = true;
  } else {
    assert(false); /* analyser checked */
  }
}
::core::any Interpreter::interpret(State& state,
                                   const ast::logic::If& iff) const {
  assert(iff.condition);
  assert(iff.true_scope);

  try {
    auto con = interpret(state, *iff.condition);

    if(any_to_bool(con)) {
      return interpret(state, *iff.true_scope);
    } else if(iff.false_scope) {
      try {
        return interpret(state, *iff.false_scope);
      } catch(std::exception&) {
        Exc<E, E::TAIL> e;
        add_exception_info(iff.token, state.file, e,
                           [&e]() { e << "In the else part defined here"; });
        std::throw_with_nested(e);
      }
    }
  } catch(std::exception&) {
    Exc<E, E::TAIL> e;
    add_exception_info(iff.token, state.file, e,
                       [&e]() { e << "In the if defined here"; });
    std::throw_with_nested(e);
  }
  return {};
}
::core::any Interpreter::interpret(State& state,
                                   const ast::loop::DoWhile& whi) const {
  assert(whi.condition);
  assert(whi.scope);

  try {
    State inner(state);
    inner.loopscope = true;
    auto ret = interpret_shared(inner, *whi.scope);

    while(!inner.returning && !inner.breaking &&
          any_to_bool(interpret(inner, *whi.condition))) {
      ret = interpret_shared(inner, *whi.scope);
    }
    if(inner.returning) {
      state.returning = true;
    }
    return ret;
  } catch(std::exception&) {
    Exc<E, E::TAIL> e;
    add_exception_info(whi.token, state.file, e,
                       [&e]() { e << "In the do/while defined here"; });
    std::throw_with_nested(e);
  }
}
::core::any Interpreter::interpret(State& state,
                                   const ast::loop::For& foor) const {
  try {
    State inner(state);
    inner.loopscope = true;
    ::core::any ret;

    if(foor.define) {
      define_variable(inner, *foor.define);
    }
    if(foor.variable) {
      interpret(inner, *foor.variable);
    }

    while(!inner.returning && !inner.breaking &&
          any_to_bool(interpret(inner, *foor.condition))) {
      ret = interpret_shared(inner, *foor.scope);
      ret = interpret(inner, *foor.operation);
    }
    if(inner.returning) {
      state.returning = true;
    }
    return ret;
  } catch(std::exception&) {
    Exc<E, E::TAIL> e;
    add_exception_info(foor.token, state.file, e,
                       [&e]() { e << "In the for defined here"; });
    std::throw_with_nested(e);
  }
}
::core::any Interpreter::interpret(State& state,
                                   const ast::loop::While& whi) const {
  assert(whi.condition);
  assert(whi.scope);

  try {
    State inner(state);
    inner.loopscope = true;
    ::core::any ret;
    while(!inner.returning && !inner.breaking &&
          any_to_bool(interpret(inner, *whi.condition))) {
      ret = interpret_shared(inner, *whi.scope);
    }
    if(inner.returning) {
      state.returning = true;
    }
    return ret;
  } catch(std::exception&) {
    Exc<E, E::TAIL> e;
    add_exception_info(whi.token, state.file, e,
                       [&e]() { e << "In the while defined here"; });
    std::throw_with_nested(e);
  }
}
::core::any Interpreter::interpret(State& state, const ast::callable::Return& ret) const {
  assert(ret.output);

  try {
    ::core::any out;
    ret.output->value.match(
        [&](const callable::Callable& o) { out = interpret(state, o); },
        [&](const Operator& o) { out = interpret(state, o); },
        [&](const Variable& o) {
          if(state.stack->owns_variable(o.token.token)) {
            state.stack->variable(
                o.token.token, [&](::core::any& var) { out = std::move(var); });
          } else {
            state.stack->variable(o.token.token,
                                  [&](const ::core::any& var) { out = var; });
          }
        },
        [&](const Literal<Literals::BOOL>& c) { out = c.data; },
        [&](const Literal<Literals::INT>& c) { out = c.data; },
        [&](const Literal<Literals::DOUBLE>& c) { out = c.data; },
        [&](const Literal<Literals::STRING>& c) { out = c.data; });
    state.returning = true;

    return out;
  } catch(std::exception&) {
    Exc<E, E::TAIL> e;
    add_exception_info(ret.token, state.file, e,
                       [&e]() { e << "In the return defined here"; });
    std::throw_with_nested(e);
  }
}
::core::any Interpreter::interpret(State& state, const Scope& scope) const {
  State inner(state);
  auto ret = interpret_shared(inner, scope);

  if(inner.breaking) {
    state.breaking = true;
  } else if(inner.returning) {
    state.returning = true;
  }
  return ret;
}
::core::any Interpreter::interpret_shared(State& state,
                                          const Scope& scope) const {
  define_functions(state, scope);

  ::core::any ret;
  for(const auto& n : scope.nodes) {
    n.match(
        [this, &state](const Define& e) { define_variable(state, e); },
        [this, &state](const Operator& e) { interpret(state, e); },
        [this, &state](const loop::Break& e) { interpret(state, e); },
        [this, &state](const Callable& e) { interpret(state, e); },
        [this, &state, &ret](const DoWhile& e) { ret = interpret(state, e); },
        [this, &state, &ret](const For& e) { ret = interpret(state, e); },
        [this, &state, &ret](const If& e) { ret = interpret(state, e); },
        [this, &state](const Literal<Literals::BOOL>&) { /* ignore */ },
        [this, &state](const Literal<Literals::DOUBLE>&) { /* ignore */ },
        [this, &state](const Literal<Literals::INT>&) { /* ignore */ },
        [this, &state](const Literal<Literals::STRING>&) { /* ignore */ },
        [this, &state, &ret](const callable::Return& e) {
          ret = interpret(state, e);
        },
        [this, &state, &ret](const Scope& e) { ret = interpret(state, e); },
        [this, &state, &ret](const While& e) { ret = interpret(state, e); },
        [this, &state](const Variable&) { /* ignore */ });

    if(state.breaking || state.returning) {
      break;
    }
  }
  return ret;
}

//////////////////////////////////////////
// interpret function
//////////////////////////////////////////
Interpreter::Arguments
Interpreter::args_from_call(State& state, const Callable& call,
                            const Arguments& command_args) const {
  Arguments args;

  for(const auto& p : call.parameter) {
    const auto& name = p.first.token.token;

    if(command_args.has(name)) {
      auto val = interpret(state, p.second);
      args.add(p.first.token.token, "macro_call", val.ref.get());
    } else {
      assert(false && "Too many arguments!");  // Should not happen
    }
  }
  return args;
}

void Interpreter::add_parameter(State& state, State& outer,
                                const ValueProducer& val,
                                const std::string& par) const {
  auto ret_par = [this](State& s, State& o, const std::string& p,
                        const auto& v) {
    s.stack->add_variable(p);
    s.stack->variable(p, [&](::core::any& var) { var = interpret(o, v); });
  };
  auto lit_par = [this](State& s, const std::string& p, const auto& v) {
    s.stack->add_variable(p);
    s.stack->variable(p, [&](::core::any& var) { var = v.data; });
  };
  auto var_par = [this](State& s, State& o, const std::string& p, auto v) {
    if(o.stack->has_variable(v.token.token)) {
      o.stack->variable(v.token.token, [&s, &p](::core::any& var) {
        s.stack->add_alias(p, var);
      });
    } else {
      assert(false); /* analyser checked */
    }
  };
  val.value.match(
      [&](const callable::Callable& o) { ret_par(state, outer, par, o); },
      [&](const Operator& o) { ret_par(state, outer, par, o); },
      [&](const Variable& o) { var_par(state, outer, par, o); },
      [&](const Literal<Literals::BOOL>& c) { lit_par(state, par, c); },
      [&](const Literal<Literals::INT>& c) { lit_par(state, par, c); },
      [&](const Literal<Literals::DOUBLE>& c) { lit_par(state, par, c); },
      [&](const Literal<Literals::STRING>& c) { lit_par(state, par, c); });
}

void Interpreter::add_parameter(State& state, State& outer,
                                const Callable& call,
                                const Function& fun) const {
  if(call.parameter.size() != fun.parameter.size()) {
    assert(false);  // Should not happen
  }
  for(const auto& p : call.parameter) {
    auto& par = p.first.token.token;
    auto it = std::find_if(
        fun.parameter.begin(), fun.parameter.end(),
        [&par](const Variable& var) { return par == var.token.token; });

    if(fun.parameter.end() != it) {
      add_parameter(state, outer, p.second, par);
    } else {
      assert(false);  // Should not happen
    }
  }
}

void Interpreter::add_arguments(State& state, Arguments& args,
                                const Function& fun) const {
  if(args.size() != fun.parameter.size()) {
    assert(false);  // Should not happen
  }
  for(const auto& p : fun.parameter) {
    if(args.has(p.token.token)) {
      state.stack->add_alias(p.token.token, args[p.token.token]);
    } else {
      assert(false);  // Should not happen
    }
  }
}

::core::any Interpreter::interpret(State& state,
                                   const ast::callable::Callable& call) const {
  ::core::any ret;
  if(state.stack->has_function(call)) {
    state.stack->function(call, [&](const Function& fun, auto stack) {
      try {
        State inner(state, std::make_shared<Stack>(std::move(stack)));
        inner.loopscope = false;

        add_parameter(inner, state, call, fun);

        ret = interpret_shared(inner, *fun.scope);
      } catch(std::exception&) {
        Exc<E, E::TAIL> e;
        add_exception_info(fun.token, state.file, e, [&e, &fun]() {
          e << "In the '" << fun.token.token << "' function defined here";
        });
        std::throw_with_nested(e);
      }
    });
  } else {
    try {
      auto com = command_provider_->get_command(state.scope, call.token.token);
      ret = com.execute(args_from_call(state, call, com.arguments()));
    } catch(...) {
      bool once = true;
      Exc<E, E::MISSING_FUNCTION> e(__FILE__, __LINE__, "Missing function");
      e << "There was no matching function '" << call.token.token << "(";
      for(const auto& p : call.parameter) {
        if(once) {
          once = false;
          e << p.first.token.token;
        } else {
          e << ", " << p.first.token.token;
        }
      }
      e << ")'.";
      throw e;
    }
  }

  return ret;
}

::core::any Interpreter::interpret_main(State& state, Arguments args) const {
  ::core::any ret;

  Callable call({0, 0, "main"});
  for(const auto& p : args) {
    call.parameter.emplace_back(Variable({0, 0, p.name()}), Variable());
  }

  state.stack->function(call, [&](const Function& fun, auto stack) {
    try {
      State inner(state, std::make_shared<Stack>(std::move(stack)));

      add_arguments(inner, args, fun);
      ret = interpret_shared(inner, *fun.scope);
    } catch(std::exception&) {
      Exc<E, E::TAIL> e;
      add_exception_info(fun.token, state.file, e, [&e, &fun]() {
        e << "In the 'main' function defined here";
      });
      std::throw_with_nested(e);
    }
  });
  return ret;
}
}
}
}
