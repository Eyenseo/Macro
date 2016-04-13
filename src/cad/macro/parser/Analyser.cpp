#include "cad/macro/parser/Analyser.h"

#include "cad/macro/parser/Message.h"
#include "cad/macro/ast/Scope.h"

#include <p3/common/signal/Signal.h>

#include <algorithm>

namespace cad {
namespace macro {
namespace parser {
namespace {

template <typename T>
using Signal = p3::common::signal::Signal<T>;

enum class SignalType { START, END };

struct Stack {
  using RV = std::reference_wrapper<const ast::Variable>;

  template <typename T>
  struct myPair {  // optional doesn't like std::pair or std::tuple ...
    std::reference_wrapper<const T> first;
    std::reference_wrapper<const T> second;
  };

  std::vector<std::reference_wrapper<const ast::Variable>> variables;
  std::vector<std::reference_wrapper<const ast::callable::Function>> functions;
  Stack* parent = nullptr;

  ::core::optional<myPair<ast::Variable>> has_double_var() const {
    if(variables.size() > 1) {
      const auto& back = variables.back().get();
      auto it = std::find_if(variables.begin(), variables.end() - 1,
                             [&back](const auto& var) {
                               return var.get().token.token == back.token.token;
                             });

      if(variables.end() - 1 != it) {
        return {{*it, back}};
      }
    }
    return {};
  }

  bool has_var(const std::string& name) const {
    if(variables.end() != std::find_if(variables.begin(), variables.end(),
                                       [&name](const auto& var) {
                                         return var.get().token.token == name;
                                       })) {
      return true;
    } else if(parent) {
      return parent->has_var(name);
    }
    return false;
  }

  ::core::optional<myPair<ast::callable::Function>> has_double_fun() const {
    if(functions.size() > 1) {
      auto& back = functions.back().get();
      auto it = std::find_if(
          functions.begin(), functions.end() - 1, [&back](const auto& fun) {
            if(fun.get().token.token == back.token.token &&
               fun.get().parameter.size() == back.parameter.size()) {
              // Parameter
              for(const auto& fp : fun.get().parameter) {
                bool found = false;
                for(const auto& cp : back.parameter) {
                  if(fp.token.token == cp.token.token) {
                    found = true;
                    break;
                  }
                }
                if(!found) {
                  return false;
                }
              }
            } else {
              return false;
            }
            return true;
          });

      if(functions.end() - 1 != it) {
        return {{*it, back}};
      }
    }
    return {};
  }
};

struct State;
struct Signals {
  Signal<void(SignalType, const State&, const ast::BinaryOperator&)> biop;
  Signal<void(SignalType, const State&, const ast::Break&)> br;
  Signal<void(SignalType, const State&, const ast::callable::Callable&)> call;
  Signal<void(SignalType, const State&, const ast::callable::EntryFunction&)>
      enfun;
  Signal<void(SignalType, const State&, const ast::callable::Function&)> fun;
  Signal<void(SignalType, const State&, const ast::Define&)> def;
  Signal<void(SignalType, const State&,
              const ast::Literal<ast::Literals::BOOL>&)> bo;
  Signal<void(SignalType, const State&,
              const ast::Literal<ast::Literals::DOUBLE>&)> dou;
  Signal<void(SignalType, const State&,
              const ast::Literal<ast::Literals::INT>&)> intt;
  Signal<void(SignalType, const State&,
              const ast::Literal<ast::Literals::STRING>&)> str;
  Signal<void(SignalType, const State&, const ast::logic::If&)> iff;
  Signal<void(SignalType, const State&, const ast::loop::DoWhile&)> dowhile;
  Signal<void(SignalType, const State&, const ast::loop::For&)> forr;
  Signal<void(SignalType, const State&, const ast::loop::While&)> whi;
  Signal<void(SignalType, const State&, const ast::Return&)> ret;
  Signal<void(SignalType, const State&, const ast::Scope&)> sco;
  Signal<void(SignalType, const State&, const ast::UnaryOperator&)> unop;
  Signal<void(SignalType, const State&, const ast::Variable&)> var;
};

struct State {
  using MessageStack = std::vector<Message>;

  Stack stack;
  std::shared_ptr<MessageStack> current_message;
  std::shared_ptr<std::vector<MessageStack>> messages;
  std::shared_ptr<Signals> signal;
  std::reference_wrapper<const ast::Scope> scope;
  std::string file;
  bool loop;
  bool root_scope;

  State(const ast::Scope& s, std::string f)
      : stack()
      , current_message(std::make_shared<MessageStack>())
      , messages(std::make_shared<std::vector<MessageStack>>())
      , signal(std::make_shared<Signals>())
      , scope(s)
      , file(std::move(f))
      , loop(false)
      , root_scope(false) {
  }
  State(State& parent, const ast::Scope& s, bool l = false)
      : stack()
      , current_message(parent.current_message)
      , messages(parent.messages)
      , signal(parent.signal)
      , scope(s)
      , file(parent.file)
      , loop(l ? l : parent.loop)
      , root_scope(parent.root_scope) {
    stack.parent = &parent.stack;
  }
};

namespace analyse {
void analyse(State& state, const ast::BinaryOperator& e);
void analyse(State& state, const ast::Break& e);
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
void analyse(State& state, const ast::Return& e);
void analyse(State& state, const ast::Scope& e);
void analyse(State& state, const ast::UnaryOperator& e);
void analyse(State& state, const ast::Variable& e);
void analyse(State& state, const ast::ValueProducer& e);

void analyse(State& state, const ast::BinaryOperator& e) {
  {
    Message m(e.token, state.file);
    m << "At the operator '" << e.token.token << "' defined here:";
    state.current_message->push_back(std::move(m));
  }
  state.signal->biop.emit(SignalType::START, state, e);

  if(e.left_operand) {
    analyse(state, *e.left_operand);
  }
  if(e.right_operand) {
    analyse(state, *e.right_operand);
  }
  state.signal->biop.emit(SignalType::END, state, e);
  state.current_message->pop_back();
}
void analyse(State& state, const ast::Break& e) {
  state.signal->br.emit(SignalType::START, state, e);
  state.signal->br.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::callable::Callable& e) {
  state.signal->call.emit(SignalType::START, state, e);
  for(const auto& p : e.parameter) {
    analyse(state, p.second);
  }
  state.signal->call.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::callable::EntryFunction& e) {
  state.signal->enfun.emit(SignalType::START, state, e);
  if(e.scope) {
    State inner(state, *e.scope);
    inner.loop = false;        // we mark a new start
    inner.root_scope = false;  // we are not part of the root - we can return
    for(const auto& p : e.parameter) {
      inner.stack.variables.push_back(p);
    }

    analyse(inner, *e.scope);
  }
  state.signal->enfun.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::callable::Function& e) {
  state.signal->fun.emit(SignalType::START, state, e);
  if(e.scope) {
    State inner(state, *e.scope);
    inner.loop = false;        // we mark a new start
    inner.root_scope = false;  // we are not part of the root - we can return
    for(const auto& p : e.parameter) {
      inner.stack.variables.push_back(p);
    }
    analyse(inner, *e.scope);
  }
  state.signal->fun.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::Define& e) {
  state.signal->def.emit(SignalType::START, state, e);
  e.definition.match(
      [&state](const ast::callable::EntryFunction& e) {
        {
          Message m(e.token, state.file);
          m << "In the 'main' function defined here:";
          state.current_message->push_back(std::move(m));
        }
        analyse(state, e);
        state.stack.functions.push_back(e);
      },
      [&state](const ast::callable::Function& e) {
        {
          Message m(e.token, state.file);
          m << "In the '" << e.token.token << "' function defined here:";
          state.current_message->push_back(std::move(m));
        }
        analyse(state, e);
        state.stack.functions.push_back(e);
      },
      [&state](const ast::Variable& e) {
        {
          Message m(e.token, state.file);
          m << "At the variable '" << e.token.token << "' defined here:";
          state.current_message->push_back(std::move(m));
        }
        state.stack.variables.push_back(e);
        // We are good - if a variable is declared twice can be checked on the
        // end signal
      });
  state.current_message->pop_back();
  state.signal->def.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::Literal<ast::Literals::BOOL>& e) {
  state.signal->bo.emit(SignalType::START, state, e);
  state.signal->bo.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::Literal<ast::Literals::DOUBLE>& e) {
  state.signal->dou.emit(SignalType::START, state, e);
  state.signal->dou.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::Literal<ast::Literals::INT>& e) {
  state.signal->intt.emit(SignalType::START, state, e);
  state.signal->intt.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::Literal<ast::Literals::STRING>& e) {
  state.signal->str.emit(SignalType::START, state, e);
  state.signal->str.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::logic::If& e) {
  {
    Message m(e.token, state.file);
    m << "In the if defined here:";
    state.current_message->push_back(std::move(m));
  }
  state.signal->iff.emit(SignalType::START, state, e);
  if(e.condition) {
    analyse(state, *e.condition);
  }
  if(e.true_scope) {
    State inner(state, *e.true_scope);
    analyse(inner, *e.true_scope);
  }
  if(e.false_scope) {
    {
      Message m(e.false_scope->token, state.file);
      m << "In the else part defined here::";
      state.current_message->push_back(std::move(m));
    }
    State inner(state, *e.false_scope);
    analyse(inner, *e.false_scope);
    if(state.current_message->size() > 0) {
      state.current_message->pop_back();
    }
  }
  state.signal->iff.emit(SignalType::END, state, e);
  state.current_message->pop_back();
}
void analyse(State& state, const ast::loop::DoWhile& e) {
  {
    Message m(e.token, state.file);
    m << "In the do-while defined here:";
    state.current_message->push_back(std::move(m));
  }
  state.signal->dowhile.emit(SignalType::START, state, e);
  if(e.condition) {
    analyse(state, *e.condition);
  }
  if(e.scope) {
    State inner(state, *e.scope, true);
    analyse(inner, *e.scope);
  }
  state.signal->dowhile.emit(SignalType::END, state, e);
  state.current_message->pop_back();
}
void analyse(State& state, const ast::loop::For& e) {
  state.signal->forr.emit(SignalType::START, state, e);
  // TODO for
  state.signal->forr.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::loop::While& e) {
  {
    Message m(e.token, state.file);
    m << "In the while defined here:";
    state.current_message->push_back(std::move(m));
  }
  state.signal->whi.emit(SignalType::START, state, e);
  if(e.condition) {
    analyse(state, *e.condition);
  }
  if(e.scope) {
    State inner(state, *e.scope, true);
    analyse(inner, *e.scope);
  }
  state.signal->whi.emit(SignalType::END, state, e);
  state.current_message->pop_back();
}
void analyse(State& state, const ast::Return& e) {
  {
    Message m(e.token, state.file);
    m << "At return defined here:";
    state.current_message->push_back(std::move(m));
  }
  state.signal->ret.emit(SignalType::START, state, e);
  if(e.output) {
    analyse(state, *e.output);
  }
  state.signal->ret.emit(SignalType::END, state, e);
  state.current_message->pop_back();
}
void analyse(State& state, const ast::Scope& e) {
  using namespace ast;

  state.signal->sco.emit(SignalType::START, state, e);
  for(const auto& n : e.nodes) {
    n.match([&state](const BinaryOperator& e) { analyse(state, e); },
            [&state](const Break& e) { analyse(state, e); },
            [&state](const callable::Callable& e) { analyse(state, e); },
            [&state](const Define& e) { analyse(state, e); },
            [&state](const Literal<Literals::BOOL>& e) { analyse(state, e); },
            [&state](const Literal<Literals::DOUBLE>& e) { analyse(state, e); },
            [&state](const Literal<Literals::INT>& e) { analyse(state, e); },
            [&state](const Literal<Literals::STRING>& e) { analyse(state, e); },
            [&state](const logic::If& e) { analyse(state, e); },
            [&state](const loop::DoWhile& e) { analyse(state, e); },
            [&state](const loop::For& e) { analyse(state, e); },
            [&state](const loop::While& e) { analyse(state, e); },
            [&state](const Return& e) { analyse(state, e); },
            [&state](const Scope& e) {
              State inner(state, e);
              analyse(inner, e);
            },
            [&state](const UnaryOperator& e) { analyse(state, e); },
            [&state](const Variable& e) { analyse(state, e); });
  }
  state.signal->sco.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::UnaryOperator& e) {
  {
    Message m(e.token, state.file);
    m << "At the operator '" << e.token.token << "' defined here:";
    state.current_message->push_back(std::move(m));
  }
  state.signal->unop.emit(SignalType::START, state, e);
  if(e.operand) {
    analyse(state, *e.operand);
  }
  state.signal->unop.emit(SignalType::END, state, e);
  state.current_message->pop_back();
}
void analyse(State& state, const ast::Variable& e) {
  state.signal->var.emit(SignalType::START, state, e);
  state.signal->var.emit(SignalType::END, state, e);
}
void analyse(State& state, const ast::ValueProducer& e) {
  using namespace ast;

  e.value.match(
      [&state](const BinaryOperator& e) { analyse(state, e); },
      [&state](const callable::Callable& e) { analyse(state, e); },
      [&state](const Literal<Literals::BOOL>& e) { analyse(state, e); },
      [&state](const Literal<Literals::DOUBLE>& e) { analyse(state, e); },
      [&state](const Literal<Literals::INT>& e) { analyse(state, e); },
      [&state](const Literal<Literals::STRING>& e) { analyse(state, e); },
      [&state](const UnaryOperator& e) { analyse(state, e); },
      [&state](const Variable& e) { analyse(state, e); });
}
}
///////////////////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////
namespace visitor {
namespace {
std::reference_wrapper<const Token> node_to_token(const ast::Scope::Node& n) {
  using namespace ast;
  using namespace loop;
  using namespace logic;
  using namespace callable;
  Token t;
  std::reference_wrapper<const Token> ret = t;

  n.match([&ret](const BinaryOperator& n) { ret = n.token; },             //
          [&ret](const Break& n) { ret = n.token; },                      //
          [&ret](const Callable& n) { ret = n.token; },                   //
          [&ret](const Define& n) { ret = n.token; },                     //
          [&ret](const DoWhile& n) { ret = n.token; },                    //
          [&ret](const For& n) { ret = n.token; },                        //
          [&ret](const If& n) { ret = n.token; },                         //
          [&ret](const Literal<Literals::BOOL>& n) { ret = n.token; },    //
          [&ret](const Literal<Literals::DOUBLE>& n) { ret = n.token; },  //
          [&ret](const Literal<Literals::INT>& n) { ret = n.token; },     //
          [&ret](const Literal<Literals::STRING>& n) { ret = n.token; },  //
          [&ret](const Return& n) { ret = n.token; },                     //
          [&ret](const Scope& n) { ret = n.token; },                      //
          [&ret](const UnaryOperator& n) { ret = n.token; },              //
          [&ret](const While& n) { ret = n.token; },                      //
          [&ret](const Variable& n) { ret = n.token; });

  return ret;
}
}

void break_and_return_last(Signals& sigs) {
  sigs.br.connect([](SignalType t, const State& s, const auto& br) {
    if(t == SignalType::START) {
      // TODO missing !=
      if(!(s.scope.get().nodes.back() == ast::Scope::Node(br))) {
        auto stack = *s.current_message;
        Message m(node_to_token(s.scope.get().nodes.back()), s.file);
        m << "Statement after break";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
  sigs.ret.connect([](SignalType t, const State& s, const auto& ret) {
    if(t == SignalType::START) {
      // TODO missing !=
      if(!(s.scope.get().nodes.back() == ast::Scope::Node(ret))) {
        auto stack = *s.current_message;
        Message m(node_to_token(s.scope.get().nodes.back()), s.file);
        m << "Statement after return";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void no_break_in_non_loop(Signals& sigs) {
  sigs.br.connect([](SignalType t, const State& s, const auto& br) {
    if(t == SignalType::START) {
      if(!s.loop) {
        auto stack = *s.current_message;
        Message m(br.token, s.file);
        m << "Break outside of loop";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void no_return_in_root(Signals& sigs) {
  sigs.ret.connect([](SignalType t, const State& s, const auto& ret) {
    if(t == SignalType::START) {
      if(s.root_scope) {
        auto stack = *s.current_message;
        Message m(ret.token, s.file);
        m << "Return statement in root scope";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void unique_callable_parameter(Signals& sigs) {
  sigs.call.connect([](SignalType t, const State& s, const auto& call) {
    if(t == SignalType::START) {
      for(auto i = call.parameter.begin(); i != call.parameter.end(); ++i) {
        for(auto j = i + 1; j != call.parameter.end(); ++j) {
          if(i->first.token.token == j->first.token.token) {
            auto stack = *s.current_message;
            Message m1(i->first.token, s.file);
            m1 << "Parameter have to be uniquely named, but '"
               << i->first.token.token << "' was defined here:";
            Message m2(j->first.token, s.file);
            m2 << "and here:";
            stack.push_back(std::move(m2));
            stack.push_back(std::move(m1));
            s.messages->push_back(std::move(stack));
          }
        }
      }
    }
  });
}
void unique_function_parameter(Signals& sigs) {
  sigs.fun.connect([](SignalType t, const State& s, const auto& fun) {
    if(t == SignalType::START) {
      for(auto i = fun.parameter.begin(); i != fun.parameter.end(); ++i) {
        for(auto j = i + 1; j != fun.parameter.end(); ++j) {
          if(i->token.token == j->token.token) {
            auto stack = *s.current_message;
            Message m1(i->token, s.file);
            m1 << "Parameter have to be uniquely named, but '" << i->token.token
               << "' was defined here:";
            Message m2(j->token, s.file);
            m2 << "and here:";
            stack.push_back(std::move(m2));
            stack.push_back(std::move(m1));
            s.messages->push_back(std::move(stack));
          }
        }
      }
    }
  });
}
void unique_main_parameter(Signals& sigs) {
  sigs.enfun.connect([](SignalType t, const State& s, const auto& enfun) {
    if(t == SignalType::START) {
      for(auto i = enfun.parameter.begin(); i != enfun.parameter.end(); ++i) {
        for(auto j = i + 1; j != enfun.parameter.end(); ++j) {
          if(i->token.token == j->token.token) {
            auto stack = *s.current_message;
            Message m1(i->token, s.file);
            m1 << "Parameter have to be uniquely named, but '" << i->token.token
               << "' was defined here:";
            Message m2(j->token, s.file);
            m2 << "and here:";
            stack.push_back(std::move(m2));
            stack.push_back(std::move(m1));
            s.messages->push_back(std::move(stack));
          }
        }
      }
    }
  });
}
void unique_main(Signals& sigs) {
  sigs.enfun.connect([](SignalType t, const State& s, const auto& enfun) {
    if(t == SignalType::START) {
      auto it = std::find_if(
          s.stack.functions.begin(), s.stack.functions.end(),
          [](const auto& fun) { return fun.get().token.token == "main"; });
      if(s.stack.functions.end() != it) {
        auto stack = *s.current_message;
        Message m1(enfun.token, s.file);
        m1 << "Redefinition of the 'main' function here:";
        Message m2(it->get().token, s.file);
        m2 << "and here:";
        stack.push_back(std::move(m2));
        stack.push_back(std::move(m1));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void main_in_root(Signals& sigs) {
  sigs.enfun.connect([](SignalType t, const State& s, const auto& enfun) {
    if(t == SignalType::START) {
      // We get the extra parent through the analyse scope method
      if(!s.root_scope) {
        auto stack = *s.current_message;
        Message m(enfun.token, s.file);
        m << "The main function has to be in the root scope";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void variable_available(Signals& sigs) {
  sigs.var.connect([](SignalType t, const State& s, const auto& var) {
    if(t == SignalType::START) {
      if(!s.stack.has_var(var.token.token)) {
        auto stack = *s.current_message;
        Message m(var.token, s.file);
        m << "Undefined variable '" << var.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void no_double_def_variable(Signals& sigs) {
  sigs.def.connect([](SignalType t, const State& s, const auto&) {
    if(t == SignalType::END) {
      if(auto var = s.stack.has_double_var()) {
        auto stack = *s.current_message;
        Message m1(var->second.get().token, s.file);
        m1 << "Redefinition of variable '" << var->second.get().token.token
           << "' here:";
        Message m2(var->first.get().token, s.file);
        m2 << "and here:";
        stack.push_back(std::move(m2));
        stack.push_back(std::move(m1));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void no_double_def_function(Signals& sigs) {
  sigs.def.connect([](SignalType t, const State& s, const auto&) {
    if(t == SignalType::END) {
      if(auto fun = s.stack.has_double_fun()) {
        auto stack = *s.current_message;
        Message m1(fun->second.get().token, s.file);
        m1 << "Redefinition of function '" << fun->second.get().token.token
           << "' here:";
        Message m2(fun->first.get().token, s.file);
        m2 << "and here:";
        stack.push_back(std::move(m2));
        stack.push_back(std::move(m1));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void bi_op_operands(Signals& sigs) {
  sigs.biop.connect([](SignalType t, const State& s, const auto& biop) {
    if(t == SignalType::END) {
      if(!biop.left_operand) {
        auto stack = *s.current_message;
        Message m(biop.token, s.file);
        m << "Missing left operand '" << biop.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
      if(!biop.right_operand) {
        auto stack = *s.current_message;
        Message m(biop.token, s.file);
        m << "Missing right operand '" << biop.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void un_op_operands(Signals& sigs) {
  sigs.unop.connect([](SignalType t, const State& s, const auto& unop) {
    if(t == SignalType::END) {
      if(!unop.operand) {
        auto stack = *s.current_message;
        Message m(unop.token, s.file);
        m << "Missing operand '" << unop.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void bi_op_operator(Signals& sigs) {
  sigs.biop.connect([](SignalType t, const State& s, const auto& biop) {
    if(t == SignalType::END) {
      if(biop.operation == ast::BinaryOperation::NONE) {
        auto stack = *s.current_message;
        Message m(biop.token, s.file);
        m << "Missing operator '" << biop.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void un_op_operator(Signals& sigs) {
  sigs.unop.connect([](SignalType t, const State& s, const auto& unop) {
    if(t == SignalType::END) {
      if(unop.operation == ast::UnaryOperation::NONE) {
        auto stack = *s.current_message;
        Message m(unop.token, s.file);
        m << "Missing operator '" << unop.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void bi_op_assign_var(Signals& sigs) {
  auto mesage = [&](const State& s, const Token& t, const char* const type) {
    auto stack = *s.current_message;
    Message m(t, s.file);
    m << "Left hand side  has to be a variable, but was a " << type << " '"
      << t.token << "'";
    stack.push_back(std::move(m));
    s.messages->push_back(std::move(stack));
  };

  sigs.biop.connect([&mesage](SignalType t, const State& s, const auto& biop) {
    if(t == SignalType::END) {
      if(biop.operation == ast::BinaryOperation::ASSIGNMENT) {
        biop.left_operand->value.match(
            [&s, &mesage](const ast::BinaryOperator& e) {
              mesage(s, e.token, "operator");
            },
            [&s, &mesage](const ast::callable::Callable& e) {
              mesage(s, e.token, "function call");
            },
            [&s, &mesage](const ast::Literal<ast::Literals::BOOL>& e) {
              mesage(s, e.token, "literal");
            },
            [&s, &mesage](const ast::Literal<ast::Literals::DOUBLE>& e) {
              mesage(s, e.token, "literal");
            },
            [&s, &mesage](const ast::Literal<ast::Literals::INT>& e) {
              mesage(s, e.token, "literal");
            },
            [&s, &mesage](const ast::Literal<ast::Literals::STRING>& e) {
              mesage(s, e.token, "literal");
            },
            [&s, &mesage](const ast::UnaryOperator& e) {
              mesage(s, e.token, "operator");
            },
            [&s](const ast::Variable&) {
              /* good */
            });
      }
    }
  });
}
void function_scope(Signals& sigs) {
  sigs.fun.connect([](SignalType t, const State& s, const auto& fun) {
    if(t == SignalType::START) {
      if(!fun.scope) {
        auto stack = *s.current_message;
        Message m(fun.token, s.file);
        m << "Missing scope '" << fun.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void main_scope(Signals& sigs) {
  sigs.enfun.connect([](SignalType t, const State& s, const auto& enfun) {
    if(t == SignalType::START) {
      if(!enfun.scope) {
        auto stack = *s.current_message;
        Message m(enfun.token, s.file);
        m << "Missing scope '" << enfun.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void if_scope(Signals& sigs) {
  sigs.iff.connect([](SignalType t, const State& s, const auto& iff) {
    if(t == SignalType::START) {
      if(!iff.true_scope) {
        auto stack = *s.current_message;
        Message m(iff.token, s.file);
        m << "Missing scope '" << iff.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void do_while_scope(Signals& sigs) {
  sigs.dowhile.connect([](SignalType t, const State& s, const auto& dowhile) {
    if(t == SignalType::START) {
      if(!dowhile.scope) {
        auto stack = *s.current_message;
        Message m(dowhile.token, s.file);
        m << "Missing scope '" << dowhile.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void while_scope(Signals& sigs) {
  sigs.whi.connect([](SignalType t, const State& s, const auto& whi) {
    if(t == SignalType::START) {
      if(!whi.scope) {
        auto stack = *s.current_message;
        Message m(whi.token, s.file);
        m << "Missing scope '" << whi.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void if_con(Signals& sigs) {
  sigs.iff.connect([](SignalType t, const State& s, const auto& iff) {
    if(t == SignalType::START) {
      if(!iff.condition) {
        auto stack = *s.current_message;
        Message m(iff.token, s.file);
        m << "Missing condition '" << iff.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void do_while_con(Signals& sigs) {
  sigs.dowhile.connect([](SignalType t, const State& s, const auto& dowhile) {
    if(t == SignalType::START) {
      if(!dowhile.condition) {
        auto stack = *s.current_message;
        Message m(dowhile.token, s.file);
        m << "Missing condition '" << dowhile.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}
void while_con(Signals& sigs) {
  sigs.whi.connect([](SignalType t, const State& s, const auto& whi) {
    if(t == SignalType::START) {
      if(!whi.condition) {
        auto stack = *s.current_message;
        Message m(whi.token, s.file);
        m << "Missing condition '" << whi.token.token << "'";
        stack.push_back(std::move(m));
        s.messages->push_back(std::move(stack));
      }
    }
  });
}

void register_checks(Signals& sigs) {
  no_break_in_non_loop(sigs);
  break_and_return_last(sigs);
  no_return_in_root(sigs);
  unique_callable_parameter(sigs);
  unique_function_parameter(sigs);
  unique_main_parameter(sigs);
  unique_main(sigs);
  main_in_root(sigs);
  variable_available(sigs);
  no_double_def_variable(sigs);
  no_double_def_function(sigs);
  bi_op_operands(sigs);  // Should not be needed - better safe than sorry
  un_op_operands(sigs);  // Should not be needed - better safe than sorry
  bi_op_operator(sigs);  // Should not be needed - better safe than sorry
  un_op_operator(sigs);  // Should not be needed - better safe than sorry
  bi_op_assign_var(sigs);
  function_scope(sigs);  // Should not be needed - better safe than sorry
  main_scope(sigs);      // Should not be needed - better safe than sorry
  if_scope(sigs);        // Should not be needed - better safe than sorry
  do_while_scope(sigs);  // Should not be needed - better safe than sorry
  while_scope(sigs);     // Should not be needed - better safe than sorry
  if_con(sigs);          // Should not be needed - better safe than sorry
  do_while_con(sigs);    // Should not be needed - better safe than sorry
  while_con(sigs);       // Should not be needed - better safe than sorry
}
}
}

std::shared_ptr<std::vector<std::vector<Message>>>
analyse(const ast::Scope& scope, std::string file) {
  State state(scope, file);
  state.root_scope = true;

  visitor::register_checks(*state.signal);
  analyse::analyse(state, scope);

  return state.messages;
}
}
}
}
