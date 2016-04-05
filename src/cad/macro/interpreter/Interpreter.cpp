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
}

struct Interpreter::State {
  std::shared_ptr<Stack> stack;
  std::string scope;
  bool breaking;
  bool loopscope;
  bool returning;

  State()
      : State(std::make_shared<Stack>(), "") {
  }
  State(std::shared_ptr<Stack> s, std::string scope)
      : stack(std::move(s))
      , breaking(false)
      , loopscope(false)
      , returning(false) {
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
                         std::shared_ptr<OperatorProvider> operator_provider)
    : command_provider_(std::move(command_provider))
    , operator_provider_(std::move(operator_provider)) {
}

::core::any Interpreter::interpret(std::string macro, Arguments args,
                                   std::string command_scope,
                                   std::string file_name) const {
  auto scope = parser::parse(macro, file_name);
  State state;
  state.scope = std::move(command_scope);

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
      // TODO throw
      throw Exc<E, E::TODO>(__FILE__, __LINE__);
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
        [this](const BinaryOperator&) {},                                    //
        [this](const Break&) {},                                             //
        [this](const Callable&) {},                                          //
        [this](const DoWhile&) {},                                           //
        [this](const For&) {},                                               //
        [this](const If&) {},                                                //
        [this](const Literal<Literals::BOOL>&) {},                           //
        [this](const Literal<Literals::DOUBLE>&) {},                         //
        [this](const Literal<Literals::INT>&) {},                            //
        [this](const Literal<Literals::STRING>&) {},                         //
        [this](const Return&) {},                                            //
        [this](const Scope&) {},                                             //
        [this](const UnaryOperator&) {},                                     //
        [this](const While&) {},                                             //
        [this](const Variable&) {}                                           //
        );                                                                   //
  }
}
void Interpreter::define_function(State& state, const Define& def) const {
  def.definition.match(
      [&](const Function& fun) {
        state.stack->add_function(fun.token.token, fun);
      },
      [&](const EntryFunction& fun) {
        state.stack->add_function(fun.token.token, fun);
      },
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
  assert(false);
  // TODO throw
}

//////////////////////////////////////////
/// Binary
//////////////////////////////////////////
::core::any Interpreter::interpret_divide(State& state,
                                          const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::DIVIDE, lhs, rhs);
}
::core::any Interpreter::interpret_multiply(State& state,
                                            const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::MULTIPLY, lhs, rhs);
}
::core::any Interpreter::interpret_modulo(State& state,
                                          const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::MODULO, lhs, rhs);
}
::core::any Interpreter::interpret_add(State& state,
                                       const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::ADD, lhs, rhs);
}
::core::any Interpreter::interpret_subtract(State& state,
                                            const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::SUBTRACT, lhs, rhs);
}
::core::any Interpreter::interpret_smaller(State& state,
                                           const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::SMALLER, lhs, rhs);
}
::core::any
Interpreter::interpret_smaller_equal(State& state,
                                     const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::SMALLER_EQUAL, lhs, rhs);
}
::core::any Interpreter::interpret_greater(State& state,
                                           const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::GREATER, lhs, rhs);
}
::core::any
Interpreter::interpret_greater_equal(State& state,
                                     const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::GREATER_EQUAL, lhs, rhs);
}
::core::any Interpreter::interpret_equal(State& state,
                                         const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::EQUAL, lhs, rhs);
}
::core::any Interpreter::interpret_not_equal(State& state,
                                             const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::NOT_EQUAL, lhs, rhs);
}
::core::any Interpreter::interpret_and(State& state,
                                       const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::AND, lhs, rhs);
}
::core::any Interpreter::interpret_or(State& state,
                                      const BinaryOperator& op) const {
  auto lhs = interpret(state, *op.left_operand);
  auto rhs = interpret(state, *op.right_operand);

  using BiOp = OperatorProvider::BinaryOperation;
  return operator_provider_->eval(BiOp::OR, lhs, rhs);
}
::core::any Interpreter::interpret_assignment(State& state,
                                              const BinaryOperator& op) const {
  ::core::any rh;
  auto ret_par = [&](State& s, const auto& v) { rh = interpret(s, v); };
  auto lit_par = [&](const auto& v) { rh = v.data; };
  auto var_par = [&](State& s, auto v) {
    if(s.stack->has_variable(v.token.token)) {
      s.stack->variable(v.token.token, [&](::core::any& var) { rh = var; });
    } else {
      assert(false);
      // TODO throw
    }
  };

  op.right_operand->value.match(
      [&](const callable::Callable& o) { ret_par(state, o); },
      [&](const UnaryOperator& o) { ret_par(state, o); },
      [&](const BinaryOperator& o) { ret_par(state, o); },
      [&](const Variable& o) { var_par(state, o); },
      [&](const Literal<Literals::BOOL>& c) { lit_par(c); },
      [&](const Literal<Literals::INT>& c) { lit_par(c); },
      [&](const Literal<Literals::DOUBLE>& c) { lit_par(c); },
      [&](const Literal<Literals::STRING>& c) { lit_par(c); });

  op.left_operand->value.match(
      [&](const callable::Callable&) { /* TODO throw */ },
      [&](const UnaryOperator&) { /* TODO throw */ },
      [&](const BinaryOperator&) { /* TODO throw */ },
      [&](const Variable& o) {
        if(!state.stack->owns_variable(o.token.token)) {
          state.stack->remove_alias(o.token.token);
          state.stack->add_variable(o.token.token);
        }
        state.stack->variable(o.token.token,
                              [&](::core::any& var) { var = rh; });
      },
      [&](const Literal<Literals::BOOL>&) { /* TODO throw */ },
      [&](const Literal<Literals::INT>&) { /* TODO throw */ },
      [&](const Literal<Literals::DOUBLE>&) { /* TODO throw */ },
      [&](const Literal<Literals::STRING>&) { /* TODO throw */ });
  return rh;
}

::core::any Interpreter::interpret(State& state,
                                   const BinaryOperator& op) const {
  switch(op.operation) {
  case BinaryOperation::NONE:
    interpret_none();  // throws
  case BinaryOperation::DIVIDE:
    return interpret_divide(state, op);
  case BinaryOperation::MULTIPLY:
    return interpret_multiply(state, op);
  case BinaryOperation::MODULO:
    return interpret_modulo(state, op);
  case BinaryOperation::ADD:
    return interpret_add(state, op);
  case BinaryOperation::SUBTRACT:
    return interpret_subtract(state, op);
  case BinaryOperation::SMALLER:
    return interpret_smaller(state, op);
  case BinaryOperation::SMALLER_EQUAL:
    return interpret_smaller_equal(state, op);
  case BinaryOperation::GREATER:
    return interpret_greater(state, op);
  case BinaryOperation::GREATER_EQUAL:
    return interpret_greater_equal(state, op);
  case BinaryOperation::EQUAL:
    return interpret_equal(state, op);
  case BinaryOperation::NOT_EQUAL:
    return interpret_not_equal(state, op);
  case BinaryOperation::AND:
    return interpret_and(state, op);
  case BinaryOperation::OR:
    return interpret_or(state, op);
  case BinaryOperation::ASSIGNMENT:
    return interpret_assignment(state, op);
  }
  assert(false && "Reached by access after free and similar");
}

//////////////////////////////////////////
/// Unary
//////////////////////////////////////////
::core::any Interpreter::interpret_not(State& state,
                                       const UnaryOperator& op) const {
  auto rhs = interpret(state, *op.operand);

  using UnOp = OperatorProvider::UnaryOperation;
  return operator_provider_->eval(UnOp::NOT, rhs);
}

::core::any Interpreter::interpret(State& state,
                                   const UnaryOperator& op) const {
  switch(op.operation) {
  case UnaryOperation::NONE:
    interpret_none();
  case UnaryOperation::NOT:
    return interpret_not(state, op);
  }
}

//////////////////////////////////////////
/// Helper
//////////////////////////////////////////
Interpreter::SmartRef<::core::any>
Interpreter::interpret(State& state, const ValueProducer& vp) const {
  SmartRef<::core::any> f;

  vp.value.match(
      [&](const callable::Callable& o) { f.value = interpret(state, o); },
      [&](const UnaryOperator& o) { f.value = interpret(state, o); },
      [&](const BinaryOperator& o) { f.value = interpret(state, o); },
      [&](const Variable& o) {
        if(state.stack->has_variable(o.token.token)) {
          return state.stack->variable(o.token.token,
                                       [&](::core::any& var) { f.ref = var; });
        } else {
          assert(false);
          // TODO throw
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
void Interpreter::interpret(State& state, const ast::Break&) const {
  if(state.loopscope) {
    state.breaking = true;
  } else {
    assert(false);
    // TODO throw
  }
}
::core::any Interpreter::interpret(State& state,
                                   const ast::logic::If& iff) const {
  assert(iff.condition);
  auto con = interpret(state, *iff.condition);

  if(any_to_bool(con)) {
    assert(iff.true_scope);
    return interpret(state, *iff.true_scope);
  } else if(iff.false_scope) {
    return interpret(state, *iff.false_scope);
  }
  return {};
}
::core::any Interpreter::interpret(State& state,
                                   const ast::loop::DoWhile& whi) const {
  assert(whi.condition);
  assert(whi.scope);

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
}
::core::any Interpreter::interpret(State&, const ast::loop::For&) const {
  // TODO
  return {};
}
::core::any Interpreter::interpret(State& state,
                                   const ast::loop::While& whi) const {
  assert(whi.condition);
  assert(whi.scope);

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
}
::core::any Interpreter::interpret(State& state, const ast::Return& ret) const {
  assert(ret.output);

  ::core::any out;
  ret.output->value.match(
      [&](const callable::Callable& o) { out = interpret(state, o); },
      [&](const UnaryOperator& o) { out = interpret(state, o); },
      [&](const BinaryOperator& o) { out = interpret(state, o); },
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
}
::core::any Interpreter::interpret(State& state, const Scope& scope) const {
  State inner(state);
  auto&& ret = interpret_shared(inner, scope);

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
        [this, &state](const BinaryOperator& e) { interpret(state, e); },
        [this, &state](const Break& e) { interpret(state, e); },
        [this, &state](const Callable& e) { interpret(state, e); },
        [this, &state, &ret](const DoWhile& e) { ret = interpret(state, e); },
        [this, &state, &ret](const For& e) { ret = interpret(state, e); },
        [this, &state, &ret](const If& e) { ret = interpret(state, e); },
        [this, &state](const Literal<Literals::BOOL>&) { /* ignore */ },
        [this, &state](const Literal<Literals::DOUBLE>&) { /* ignore */ },
        [this, &state](const Literal<Literals::INT>&) { /* ignore */ },
        [this, &state](const Literal<Literals::STRING>&) { /* ignore */ },
        [this, &state, &ret](const Return& e) { ret = interpret(state, e); },
        [this, &state, &ret](const Scope& e) { ret = interpret(state, e); },
        [this, &state](const UnaryOperator& e) { interpret(state, e); },
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
      // TODO throw
      assert(false && "Too many arguments!");
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
      assert(false);
      // TODO throw
    }
  };
  val.value.match(
      [&](const callable::Callable& o) { ret_par(state, outer, par, o); },
      [&](const UnaryOperator& o) { ret_par(state, outer, par, o); },
      [&](const BinaryOperator& o) { ret_par(state, outer, par, o); },
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
    assert(false);
    // TODO throw
  }
  for(const auto& p : call.parameter) {
    auto& par = p.first.token.token;
    auto it = std::find_if(
        fun.parameter.begin(), fun.parameter.end(),
        [&par](const Variable& var) { return par == var.token.token; });

    if(fun.parameter.end() != it) {
      add_parameter(state, outer, p.second, par);
    } else {
      // parameter that is not an parameter
      assert(false);
      // TODO throw
    }
  }
}

void Interpreter::add_arguments(State& state, Arguments& args,
                                const Function& fun) const {
  if(args.size() != fun.parameter.size()) {
    assert(false);
    // TODO throw
  }
  for(const auto& p : fun.parameter) {
    if(args.has(p.token.token)) {
      state.stack->add_alias(p.token.token, args[p.token.token]);
    } else {
      assert(false);
      // TODO throw
    }
  }
}

::core::any Interpreter::interpret(State& state,
                                   const ast::callable::Callable& call) const {
  ::core::any ret;

  if(state.stack->has_function(call.token.token)) {
    state.stack->function(
        call.token.token, [&](const Function& fun, auto stack) {
          State inner(std::make_shared<Stack>(std::move(stack)), state.scope);

          add_parameter(inner, state, call, fun);

          ret = interpret(inner, *fun.scope);
        });
  } else {
    try {
      auto com = command_provider_->get_command(state.scope, call.token.token);
      ret = com.execute(args_from_call(state, call, com.arguments()));
    } catch(...) {
      assert(false);
      // TODO throw
    }
  }
  return ret;
}

::core::any Interpreter::interpret_main(State& state, Arguments args) const {
  ::core::any ret;

  if(state.stack->has_function("main")) {
    state.stack->function("main", [&](const Function& fun, auto stack) {
      State inner(std::move(stack), state.scope);

      add_arguments(inner, args, fun);
      ret = interpret(inner, *fun.scope);
    });
  } else {
    assert(false);
    // TODO throw
  }
  return ret;
}
}
}
}
