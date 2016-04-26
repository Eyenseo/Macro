#include "cad/macro/parser/Analyser.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/parser/Message.h"
#include "cad/macro/parser/analyser/State.h"

#include <algorithm>
#include <cassert>

namespace cad {
namespace macro {
namespace parser {
namespace {
template <typename T>
using Signal = p3::common::signal::Signal<T>;
using SignalType = Analyser::SignalType;
using State = analyser::State;

std::reference_wrapper<const Token> node_to_token(const ast::Scope::Node& n) {
  using namespace ast;
  using namespace loop;
  using namespace logic;
  using namespace callable;
  Token t;
  std::reference_wrapper<const Token> ret = t;

  n.match([&ret](const Operator& n) { ret = n.token; },                   //
          [&ret](const Continue& n) { ret = n.token; },                   //
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
          [&ret](const While& n) { ret = n.token; },                      //
          [&ret](const Variable& n) { ret = n.token; });

  return ret;
}
}

//////////////////////////////////////////
// Walker
//////////////////////////////////////////
void Analyser::analyse(State& state, const ast::Operator& e) {
  {
    Message m(e.token, file_);
    m << "At the operator '" << e.token.token << "' defined here";
    current_message_.push_back(std::move(m));
  }
  biop.emit(*this, SignalType::START, state, e);

  if(e.left_operand) {
    analyse(state, *e.left_operand);
  }
  if(e.right_operand) {
    analyse(state, *e.right_operand);
  }
  biop.emit(*this, SignalType::END, state, e);
  current_message_.pop_back();
}
void Analyser::analyse(State& state, const ast::loop::Break& e) {
  br.emit(*this, SignalType::START, state, e);
  br.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state, const ast::loop::Continue& e) {
  con.emit(*this, SignalType::START, state, e);
  con.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state, const ast::callable::Callable& e) {
  call.emit(*this, SignalType::START, state, e);
  for(const auto& p : e.parameter) {
    analyse(state, p.second);
  }
  call.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state, const ast::callable::EntryFunction& e) {
  enfun.emit(*this, SignalType::START, state, e);
  if(e.scope) {
    State inner(state, *e.scope);
    inner.loop = false;        // we mark a new start
    inner.root_scope = false;  // we are not part of the root - we can return
    for(const auto& p : e.parameter) {
      inner.stack.variables.push_back(p);
    }

    analyse(inner, *e.scope);
  }
  enfun.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state, const ast::callable::Function& e) {
  fun.emit(*this, SignalType::START, state, e);
  if(e.scope) {
    State inner(state, *e.scope);
    inner.loop = false;        // we mark a new start
    inner.root_scope = false;  // we are not part of the root - we can return
    for(const auto& p : e.parameter) {
      inner.stack.variables.push_back(p);
    }
    analyse(inner, *e.scope);
  }
  fun.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state, const ast::Define& e) {
  def.emit(*this, SignalType::START, state, e);
  e.definition.match(
      [this, &state](const ast::callable::EntryFunction& e) {
        {
          Message m(e.token, file_);
          m << "In the 'main' function defined here";
          current_message_.push_back(std::move(m));
        }
        analyse(state, e);
        state.stack.functions.push_back(e);
      },
      [this, &state](const ast::callable::Function& e) {
        {
          Message m(e.token, file_);
          m << "In the '" << e.token.token << "' function defined here";
          current_message_.push_back(std::move(m));
        }
        analyse(state, e);
        state.stack.functions.push_back(e);
      },
      [this, &state](const ast::Variable& e) {
        {
          Message m(e.token, file_);
          m << "At the variable '" << e.token.token << "' defined here";
          current_message_.push_back(std::move(m));
        }
        state.stack.variables.push_back(e);
        // We are good - if a variable is declared twice can be checked on the
        // end signal
      });
  current_message_.pop_back();
  def.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state,
                       const ast::Literal<ast::Literals::BOOL>& e) {
  bo.emit(*this, SignalType::START, state, e);
  bo.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state,
                       const ast::Literal<ast::Literals::DOUBLE>& e) {
  dou.emit(*this, SignalType::START, state, e);
  dou.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state,
                       const ast::Literal<ast::Literals::INT>& e) {
  intt.emit(*this, SignalType::START, state, e);
  intt.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state,
                       const ast::Literal<ast::Literals::STRING>& e) {
  str.emit(*this, SignalType::START, state, e);
  str.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state, const ast::logic::If& e) {
  {
    Message m(e.token, file_);
    m << "In the if defined here";
    current_message_.push_back(std::move(m));
  }
  iff.emit(*this, SignalType::START, state, e);
  if(e.condition) {
    analyse(state, *e.condition);
  }
  if(e.true_scope) {
    State inner(state, *e.true_scope);
    analyse(inner, *e.true_scope);
  }
  if(e.false_scope) {
    {
      Message m(e.false_scope->token, file_);
      m << "In the else part defined here";
      current_message_.push_back(std::move(m));
    }
    State inner(state, *e.false_scope);
    analyse(inner, *e.false_scope);
    if(current_message_.size() > 0) {
      current_message_.pop_back();
    }
  }
  iff.emit(*this, SignalType::END, state, e);
  current_message_.pop_back();
}
void Analyser::analyse(State& state, const ast::loop::DoWhile& e) {
  {
    Message m(e.token, file_);
    m << "In the do-while defined here";
    current_message_.push_back(std::move(m));
  }
  dowhile.emit(*this, SignalType::START, state, e);
  if(e.condition) {
    analyse(state, *e.condition);
  }
  if(e.scope) {
    State inner(state, *e.scope, true);
    analyse(inner, *e.scope);
  }
  dowhile.emit(*this, SignalType::END, state, e);
  current_message_.pop_back();
}
void Analyser::analyse(State& state, const ast::loop::For& e) {
  {
    Message m(e.token, file_);
    m << "In the do-while defined here";
    current_message_.push_back(std::move(m));
  }
  forr.emit(*this, SignalType::START, state, e);
  State inner(state, *e.scope, true);
  if(e.define) {
    analyse(inner, *e.define);
  }
  if(e.variable) {
    analyse(inner, *e.variable);
  }
  if(e.condition) {
    analyse(inner, *e.condition);
  }
  if(e.operation) {
    analyse(inner, *e.operation);
  }
  if(e.scope) {
    analyse(inner, *e.scope);
  }
  forr.emit(*this, SignalType::END, state, e);
  current_message_.pop_back();
}
void Analyser::analyse(State& state, const ast::loop::While& e) {
  {
    Message m(e.token, file_);
    m << "In the while defined here";
    current_message_.push_back(std::move(m));
  }
  whi.emit(*this, SignalType::START, state, e);
  if(e.condition) {
    analyse(state, *e.condition);
  }
  if(e.scope) {
    State inner(state, *e.scope, true);
    analyse(inner, *e.scope);
  }
  whi.emit(*this, SignalType::END, state, e);
  current_message_.pop_back();
}
void Analyser::analyse(State& state, const ast::callable::Return& e) {
  {
    Message m(e.token, file_);
    m << "At return defined here";
    current_message_.push_back(std::move(m));
  }
  ret.emit(*this, SignalType::START, state, e);
  if(e.output) {
    analyse(state, *e.output);
  }
  ret.emit(*this, SignalType::END, state, e);
  current_message_.pop_back();
}
void Analyser::analyse(State& state, const ast::Scope& e) {
  using namespace ast;

  sco.emit(*this, SignalType::START, state, e);
  for(const auto& n : e.nodes) {
    n.match(
        [this, &state](const Operator& e) { analyse(state, e); },
        [this, &state](const loop::Continue& e) { analyse(state, e); },
        [this, &state](const loop::Break& e) { analyse(state, e); },
        [this, &state](const callable::Callable& e) { analyse(state, e); },
        [this, &state](const Define& e) { analyse(state, e); },
        [this, &state](const Literal<Literals::BOOL>& e) { analyse(state, e); },
        [this, &state](const Literal<Literals::DOUBLE>& e) {
          analyse(state, e);
        },
        [this, &state](const Literal<Literals::INT>& e) { analyse(state, e); },
        [this, &state](const Literal<Literals::STRING>& e) {
          analyse(state, e);
        },
        [this, &state](const logic::If& e) { analyse(state, e); },
        [this, &state](const loop::DoWhile& e) { analyse(state, e); },
        [this, &state](const loop::For& e) { analyse(state, e); },
        [this, &state](const loop::While& e) { analyse(state, e); },
        [this, &state](const callable::Return& e) { analyse(state, e); },
        [this, &state](const Scope& e) {
          State inner(state, e);
          analyse(inner, e);
        },
        [this, &state](const Variable& e) { analyse(state, e); });
  }
  sco.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state, const ast::Variable& e) {
  var.emit(*this, SignalType::START, state, e);
  var.emit(*this, SignalType::END, state, e);
}
void Analyser::analyse(State& state, const ast::ValueProducer& e) {
  using namespace ast;

  e.value.match(
      [this, &state](const Operator& e) { analyse(state, e); },
      [this, &state](const callable::Callable& e) { analyse(state, e); },
      [this, &state](const Literal<Literals::BOOL>& e) { analyse(state, e); },
      [this, &state](const Literal<Literals::DOUBLE>& e) { analyse(state, e); },
      [this, &state](const Literal<Literals::INT>& e) { analyse(state, e); },
      [this, &state](const Literal<Literals::STRING>& e) { analyse(state, e); },
      [this, &state](const Variable& e) { analyse(state, e); });
}

//////////////////////////////////////////
// Visitors
//////////////////////////////////////////
void Analyser::break_last_node() {
  br.connect([](Analyser& ana, SignalType t, const State& s, const auto& br) {
    if(t == SignalType::START) {
      // TODO missing !=
      if(!(s.scope.get().nodes.back() == ast::Scope::Node(br))) {
        auto stack = ana.current_message_;
        Message m(node_to_token(s.scope.get().nodes.back()), ana.file_);
        m << "Statement after break";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::continue_last_node() {
  con.connect([](Analyser& ana, SignalType t, const State& s, const auto& con) {
    if(t == SignalType::START) {
      // TODO missing !=
      if(!(s.scope.get().nodes.back() == ast::Scope::Node(con))) {
        auto stack = ana.current_message_;
        Message m(node_to_token(s.scope.get().nodes.back()), ana.file_);
        m << "Statement after continue";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::return_last_node() {
  ret.connect([](Analyser& ana, SignalType t, const State& s, const auto& ret) {
    if(t == SignalType::START) {
      // TODO missing !=
      if(!(s.scope.get().nodes.back() == ast::Scope::Node(ret))) {
        auto stack = ana.current_message_;
        Message m(node_to_token(s.scope.get().nodes.back()), ana.file_);
        m << "Statement after return";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::no_break_in_non_loop() {
  br.connect([](Analyser& ana, SignalType t, const State& s, const auto& br) {
    if(t == SignalType::START) {
      if(!s.loop) {
        auto stack = ana.current_message_;
        Message m(br.token, ana.file_);
        m << "Break outside of loop";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::no_continue_in_non_loop() {
  con.connect([](Analyser& ana, SignalType t, const State& s, const auto& con) {
    if(t == SignalType::START) {
      if(!s.loop) {
        auto stack = ana.current_message_;
        Message m(con.token, ana.file_);
        m << "Continue outside of loop";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::no_return_in_root() {
  ret.connect([](Analyser& ana, SignalType t, const State& s, const auto& ret) {
    if(t == SignalType::START) {
      if(s.root_scope) {
        auto stack = ana.current_message_;
        Message m(ret.token, ana.file_);
        m << "Return statement in root scope";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::unique_callable_parameter() {
  call.connect([](Analyser& ana, SignalType t, const State&, const auto& call) {
    if(t == SignalType::START) {
      for(auto i = call.parameter.begin(); i != call.parameter.end(); ++i) {
        for(auto j = i + 1; j != call.parameter.end(); ++j) {
          if(i->first.token.token == j->first.token.token) {
            auto stack = ana.current_message_;
            Message m1(i->first.token, ana.file_);
            m1 << "Parameter have to be uniquely named, but '"
               << i->first.token.token << "' was defined here";
            Message m2(j->first.token, ana.file_);
            m2 << "and here";
            stack.push_back(std::move(m2));
            stack.push_back(std::move(m1));
            ana.messages_.push_back(std::move(stack));
          }
        }
      }
    }
  });
}
void Analyser::unique_function_parameter() {
  fun.connect([](Analyser& ana, SignalType t, const State&, const auto& fun) {
    if(t == SignalType::START) {
      for(auto i = fun.parameter.begin(); i != fun.parameter.end(); ++i) {
        for(auto j = i + 1; j != fun.parameter.end(); ++j) {
          if(i->token.token == j->token.token) {
            auto stack = ana.current_message_;
            Message m1(i->token, ana.file_);
            m1 << "Parameter have to be uniquely named, but '" << i->token.token
               << "' was defined here";
            Message m2(j->token, ana.file_);
            m2 << "and here";
            stack.push_back(std::move(m2));
            stack.push_back(std::move(m1));
            ana.messages_.push_back(std::move(stack));
          }
        }
      }
    }
  });
}
void Analyser::unique_main_parameter() {
  enfun.connect([](Analyser& ana, SignalType t, const State&,
                   const auto& enfun) {
    if(t == SignalType::START) {
      for(auto i = enfun.parameter.begin(); i != enfun.parameter.end(); ++i) {
        for(auto j = i + 1; j != enfun.parameter.end(); ++j) {
          if(i->token.token == j->token.token) {
            auto stack = ana.current_message_;
            Message m1(i->token, ana.file_);
            m1 << "Parameter have to be uniquely named, but '" << i->token.token
               << "' was defined here";
            Message m2(j->token, ana.file_);
            m2 << "and here";
            stack.push_back(std::move(m2));
            stack.push_back(std::move(m1));
            ana.messages_.push_back(std::move(stack));
          }
        }
      }
    }
  });
}
void Analyser::unique_main() {
  enfun.connect(
      [](Analyser& ana, SignalType t, const State& s, const auto& enfun) {
        if(t == SignalType::START) {
          auto it = std::find_if(
              s.stack.functions.begin(), s.stack.functions.end(),
              [](const auto& fun) { return fun.get().token.token == "main"; });
          if(s.stack.functions.end() != it) {
            auto stack = ana.current_message_;
            Message m1(enfun.token, ana.file_);
            m1 << "Redefinition of the 'main' function here";
            Message m2(it->get().token, ana.file_);
            m2 << "and here";
            stack.push_back(std::move(m2));
            stack.push_back(std::move(m1));
            ana.messages_.push_back(std::move(stack));
          }
        }
      });
}
void Analyser::main_in_root() {
  enfun.connect(
      [](Analyser& ana, SignalType t, const State& s, const auto& enfun) {
        if(t == SignalType::START) {
          // We get the extra parent through the analyse scope method
          if(!s.root_scope) {
            auto stack = ana.current_message_;
            Message m(enfun.token, ana.file_);
            m << "The main function has to be in the root scope";
            stack.push_back(std::move(m));
            ana.messages_.push_back(std::move(stack));
          }
        }
      });
}
void Analyser::variable_available() {
  var.connect([](Analyser& ana, SignalType t, const State& s, const auto& var) {
    if(t == SignalType::START) {
      if(!s.stack.has_var(var.token.token)) {
        auto stack = ana.current_message_;
        Message m(var.token, ana.file_);
        m << "Undefined variable '" << var.token.token << "'";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::no_double_def_variable() {
  def.connect([](Analyser& ana, SignalType t, const State& s, const auto&) {
    if(t == SignalType::END) {
      if(auto var = s.stack.has_double_var()) {
        auto stack = ana.current_message_;
        Message m1(var->second.get().token, ana.file_);
        m1 << "Redefinition of variable '" << var->second.get().token.token
           << "' here";
        Message m2(var->first.get().token, ana.file_);
        m2 << "and here";
        stack.push_back(std::move(m2));
        stack.push_back(std::move(m1));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::no_double_def_function() {
  def.connect([](Analyser& ana, SignalType t, const State& s, const auto&) {
    if(t == SignalType::END) {
      if(auto fun = s.stack.has_double_fun()) {
        auto stack = ana.current_message_;
        Message m1(fun->second.get().token, ana.file_);
        m1 << "Redefinition of function '" << fun->second.get().token.token
           << "' here";
        Message m2(fun->first.get().token, ana.file_);
        m2 << "and here";
        stack.push_back(std::move(m2));
        stack.push_back(std::move(m1));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::op_operands() {
  biop.connect([](Analyser& ana, SignalType t, const State&, const auto& biop) {
    if(t == SignalType::END) {
      if(biop.operation != ast::Operation::NOT &&
         biop.operation != ast::Operation::PRINT &&
         biop.operation != ast::Operation::TYPEOF &&
         biop.operation != ast::Operation::NEGATIVE &&
         biop.operation != ast::Operation::POSITIVE && !biop.left_operand) {
        auto stack = ana.current_message_;
        Message m(biop.token, ana.file_);
        m << "Missing left operand '" << biop.token.token << "'";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
      if(!biop.right_operand) {
        auto stack = ana.current_message_;
        Message m(biop.token, ana.file_);
        m << "Missing right operand '" << biop.token.token << "'";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::op_operator() {
  biop.connect([](Analyser& ana, SignalType t, const State&, const auto& biop) {
    if(t == SignalType::END) {
      if(biop.operation == ast::Operation::NONE) {
        auto stack = ana.current_message_;
        Message m(biop.token, ana.file_);
        m << "Missing operator '" << biop.token.token << "'";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::op_assign_var() {
  auto message = [](Analyser& ana, const Token& t, const char* const type) {
    auto stack = ana.current_message_;
    Message m(t, ana.file_);
    m << "Left hand side  has to be a variable, but was a " << type << " '"
      << t.token << "'";
    stack.push_back(std::move(m));
    ana.messages_.push_back(std::move(stack));
  };

  biop.connect(
      [&message](Analyser& ana, SignalType t, const State&, const auto& biop) {
        if(t == SignalType::END) {
          if(biop.operation == ast::Operation::ASSIGNMENT) {
            biop.left_operand->value.match(
                [&message, &ana](const ast::Operator& e) {
                  message(ana, e.token, "operator");
                },
                [&message, &ana](const ast::callable::Callable& e) {
                  message(ana, e.token, "function call");
                },
                [&message, &ana](const ast::Literal<ast::Literals::BOOL>& e) {
                  message(ana, e.token, "literal");
                },
                [&message, &ana](const ast::Literal<ast::Literals::DOUBLE>& e) {
                  message(ana, e.token, "literal");
                },
                [&message, &ana](const ast::Literal<ast::Literals::INT>& e) {
                  message(ana, e.token, "literal");
                },
                [&message, &ana](const ast::Literal<ast::Literals::STRING>& e) {
                  message(ana, e.token, "literal");
                },
                [](const ast::Variable&) {
                  /* good */
                });
          }
        }
      });
}
void Analyser::function_scope() {
  fun.connect([](Analyser& ana, SignalType t, const State&, const auto& fun) {
    if(t == SignalType::START) {
      if(!fun.scope) {
        auto stack = ana.current_message_;
        Message m(fun.token, ana.file_);
        m << "Missing scope '" << fun.token.token << "'";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::main_scope() {
  enfun.connect(
      [](Analyser& ana, SignalType t, const State&, const auto& enfun) {
        if(t == SignalType::START) {
          if(!enfun.scope) {
            auto stack = ana.current_message_;
            Message m(enfun.token, ana.file_);
            m << "Missing scope '" << enfun.token.token << "'";
            stack.push_back(std::move(m));
            ana.messages_.push_back(std::move(stack));
          }
        }
      });
}
void Analyser::if_scope() {
  iff.connect([](Analyser& ana, SignalType t, const State&, const auto& iff) {
    if(t == SignalType::START) {
      if(!iff.true_scope) {
        auto stack = ana.current_message_;
        Message m(iff.token, ana.file_);
        m << "Missing scope '" << iff.token.token << "'";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::do_while_scope() {
  dowhile.connect(
      [](Analyser& ana, SignalType t, const State&, const auto& dowhile) {
        if(t == SignalType::START) {
          if(!dowhile.scope) {
            auto stack = ana.current_message_;
            Message m(dowhile.token, ana.file_);
            m << "Missing scope '" << dowhile.token.token << "'";
            stack.push_back(std::move(m));
            ana.messages_.push_back(std::move(stack));
          }
        }
      });
}
void Analyser::while_scope() {
  whi.connect([](Analyser& ana, SignalType t, const State&, const auto& whi) {
    if(t == SignalType::START) {
      if(!whi.scope) {
        auto stack = ana.current_message_;
        Message m(whi.token, ana.file_);
        m << "Missing scope '" << whi.token.token << "'";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::if_con() {
  iff.connect([](Analyser& ana, SignalType t, const State&, const auto& iff) {
    if(t == SignalType::START) {
      if(!iff.condition) {
        auto stack = ana.current_message_;
        Message m(iff.token, ana.file_);
        m << "Missing condition '" << iff.token.token << "'";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}
void Analyser::do_while_con() {
  dowhile.connect(
      [](Analyser& ana, SignalType t, const State&, const auto& dowhile) {
        if(t == SignalType::START) {
          if(!dowhile.condition) {
            auto stack = ana.current_message_;
            Message m(dowhile.token, ana.file_);
            m << "Missing condition '" << dowhile.token.token << "'";
            stack.push_back(std::move(m));
            ana.messages_.push_back(std::move(stack));
          }
        }
      });
}
void Analyser::while_con() {
  whi.connect([](Analyser& ana, SignalType t, const State&, const auto& whi) {
    if(t == SignalType::START) {
      if(!whi.condition) {
        auto stack = ana.current_message_;
        Message m(whi.token, ana.file_);
        m << "Missing condition '" << whi.token.token << "'";
        stack.push_back(std::move(m));
        ana.messages_.push_back(std::move(stack));
      }
    }
  });
}

//////////////////////////////////////////
// Interface
//////////////////////////////////////////
Analyser::Analyser(std::string file)
    : file_(std::move(file)) {
  no_break_in_non_loop();
  no_continue_in_non_loop();
  break_last_node();
  continue_last_node();
  return_last_node();
  no_return_in_root();
  unique_callable_parameter();
  unique_function_parameter();
  unique_main_parameter();
  unique_main();
  main_in_root();
  variable_available();
  no_double_def_variable();
  no_double_def_function();
  op_assign_var();
  op_operands();     // Should not be needed - better safe than sorry
  op_operator();     // Should not be needed - better safe than sorry
  function_scope();  // Should not be needed - better safe than sorry
  main_scope();      // Should not be needed - better safe than sorry
  if_scope();        // Should not be needed - better safe than sorry
  do_while_scope();  // Should not be needed - better safe than sorry
  while_scope();     // Should not be needed - better safe than sorry
  if_con();          // Should not be needed - better safe than sorry
  do_while_con();    // Should not be needed - better safe than sorry
  while_con();       // Should not be needed - better safe than sorry
}

std::vector<std::vector<Message>> Analyser::analyse(const ast::Scope& scope) {
  State state(scope);
  state.root_scope = true;

  analyse(state, scope);

  current_message_.clear();
  return std::move(messages_);
}
}
}
}
