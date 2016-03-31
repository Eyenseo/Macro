#include "cad/macro/parser/Parser.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/Literal.h"
#include "cad/macro/ast/ValueProducer.h"
#include "cad/macro/parser/Tokenizer.h"

#include <exception.h>
#include <core/optional.hpp>

#include <string>
#include <regex>
#include <cassert>

namespace cad {
namespace macro {
namespace parser {
namespace {
using ConversionExc = Exc<InternalE, InternalE::BAD_CONVERSION>;
using OperatorExc = Exc<InternalE, InternalE::MISSING_OPERATOR>;
using UserSourceExc = Exc<UserE, UserE::SOURCE>;
using UserTailExc = Exc<UserE, UserE::TAIL>;

struct Tokens {
  const std::vector<Token>& tokens;
  const std::string file;

  const Token& at(size_t i) const {
    return tokens.at(i);
  }

  size_t size() const {
    return tokens.size();
  }
};

static std::vector<std::string> keywords{"if",    "else",   "do",   "while",
                                         "for",   "var",    "def",  "main",
                                         "break", "return", "true", "false"};

void parse_scope_internals(const Tokens& tokens, size_t& token,
                           ast::Scope& scope);
core::optional<ast::ValueProducer> parse_condition(const Tokens& tokens,
                                                   size_t& token);

Token node_to_token(const ast::Scope::Node& node) {
  Token token;
  node.match(
      [&token](const ast::Break& e) { token = e.token; },
      [&token](const ast::Variable& e) { token = e.token; },
      [&token](const ast::Define& e) { token = e.token; },
      [&token](const ast::callable::EntryFunction& e) { token = e.token; },
      [&token](const ast::callable::Callable& e) { token = e.token; },
      [&token](const ast::callable::Function& e) { token = e.token; },
      [&token](const ast::Return& e) { token = e.token; },
      [&token](const ast::Scope& e) { token = e.token; },
      [&token](const ast::UnaryOperator& e) { token = e.token; },
      [&token](const ast::BinaryOperator& e) { token = e.token; },
      [&token](const ast::logic::If& e) { token = e.token; },
      [&token](const ast::loop::While& e) { token = e.token; },
      [&token](const ast::loop::DoWhile& e) { token = e.token; },
      [&token](const ast::loop::For& e) { token = e.token; },
      [&token](const ast::Literal<ast::Literals::BOOL>& e) { token = e.token; },
      [&token](const ast::Literal<ast::Literals::INT>& e) { token = e.token; },
      [&token](const ast::Literal<ast::Literals::DOUBLE>& e) {
        token = e.token;
      },
      [&token](const ast::Literal<ast::Literals::STRING>& e) {
        token = e.token;
      });
  return token;
}


template <typename FUN>
void add_exception_info(const Token& token, const std::string& file,
                        ExceptionBase<UserE>& e, FUN fun) {
  e << file << ':' << token.line << ':' << token.column << ": ";
  fun();
  if(token.source_line) {
    e << '\n' << *token.source_line << '\n'
      << std::string(token.column - 1, ' ') << "^";
  }
}

template <typename FUN>
void add_exception_info_end(const Token& token, const std::string& file,
                            ExceptionBase<UserE>& e, FUN fun) {
  e << file << ':' << token.line << ':' << token.column << ": ";
  fun();
  if(token.source_line) {
    e << '\n' << *token.source_line << '\n'
      << std::string(token.column + token.token.size() - 1, ' ') << "^";
  }
}

template <typename FUN>
void add_exception_info(const Tokens& tokens, const size_t token,
                        ExceptionBase<UserE>& e, FUN fun) {
  if(token == tokens.size()) {
    add_exception_info_end(tokens.at(token - 1), tokens.file, e, fun);
  } else {
    add_exception_info(tokens.at(token), tokens.file, e, fun);
  }
}

void expect_token(const Tokens& tokens, size_t& token,
                  const char* const token_literal) {
  if(token >= tokens.size() || tokens.at(token).token != token_literal) {
    UserSourceExc e;
    add_exception_info(tokens, token, e,
                       [&] { e << "Missing '" << token_literal << "'"; });
    throw e;
  }
  ++token;
}

bool read_token(const Tokens& tokens, size_t& token,
                const char* const token_literal) {
  if(token >= tokens.size() || tokens.at(token).token != token_literal) {
    return false;
  }
  ++token;
  return true;
}

bool read_token(const Tokens& tokens, size_t& token,
                const std::regex& token_regex) {
  std::smatch match;
  if(token >= tokens.size() ||
     !std::regex_match(tokens.at(token).token, match, token_regex)) {
    return false;
  }
  ++token;
  return true;
}

core::optional<ast::Literal<ast::Literals::BOOL>>
parse_literal_bool(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  if(read_token(tokens, tmp, "true")) {
    ast::Literal<ast::Literals::BOOL> lit(tokens.at(token));
    lit.data = true;

    token = tmp;
    return lit;
  } else if(read_token(tokens, tmp, "false")) {
    ast::Literal<ast::Literals::BOOL> lit(tokens.at(token));
    lit.data = false;

    token = tmp;
    return lit;
  }
  return {};
}

core::optional<ast::Literal<ast::Literals::INT>>
parse_literal_int(const Tokens& tokens, size_t& token) {
  const static std::regex regex("([0-9]+)");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    ast::Literal<ast::Literals::INT> lit(tokens.at(token));
    lit.data = std::stoi(tokens.at(token).token);

    token = tmp;
    return lit;
  }
  return {};
}

core::optional<ast::Literal<ast::Literals::DOUBLE>>
parse_literal_double(const Tokens& tokens, size_t& token) {
  const static std::regex regex("([0-9]?.[0-9]+)");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    ast::Literal<ast::Literals::DOUBLE> lit(tokens.at(token));
    lit.data = std::stod(lit.token.token);

    token = tmp;
    return lit;
  }
  return {};
}

void unescape_string(std::string& s) {
  auto replace_all = [&s](const std::string& search,
                          const std::string& replace) {
    for(size_t pos = 0;; pos += replace.length()) {
      pos = s.find(search, pos);

      if(pos == std::string::npos) {
        break;
      }

      s.erase(pos, search.length());
      s.insert(pos, replace);
    }
  };

  replace_all("\\/", "/");
  replace_all("\\\"", "\"");
  replace_all("\\\\", "\\");
  replace_all("\\b", "\b");
  replace_all("\\f", "\f");
  replace_all("\\n", "\n");
  replace_all("\\r", "\r");
  replace_all("\\t", "\t");
  replace_all("\\a", "\a");
}

core::optional<ast::Literal<ast::Literals::STRING>>
parse_literal_string(const Tokens& tokens, size_t& token) {
  const static std::regex regex("(\\\".*\\\")");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    ast::Literal<ast::Literals::STRING> lit(tokens.at(token));
    lit.data = lit.token.token.substr(1, lit.token.token.size() - 2);

    unescape_string(lit.data);

    token = tmp;
    return lit;
  }
  return {};
}

core::optional<ast::Scope> parse_scope(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  if(read_token(tokens, tmp, "{")) {
    ast::Scope sub_scope(tokens.at(token));

    parse_scope_internals(tokens, tmp, sub_scope);
    expect_token(tokens, tmp, "}");

    token = tmp;
    return sub_scope;
  }
  return {};
}

bool is_keyword(const std::string& token) {
  for(const auto& w : keywords) {
    if(token == w) {
      return true;
    }
  }
  return false;
}

core::optional<ast::Variable> parse_variable(const Tokens& tokens,
                                             size_t& token) {
  const static std::regex regex("([a-z][a-z0-9_]*)");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    ast::Variable var(Token(tokens.at(token)));
    if(is_keyword(var.token.token)) {
      UserSourceExc e;
      add_exception_info(tokens, token, e, [&] {
        e << "'" << var.token.token
          << "' is a keyword an may not be used as qualifier.";
      });
      throw e;
    }
    token = tmp;
    return var;
  }
  return {};
}

template <typename T,
          typename std::enable_if<
              std::is_same<T, ast::callable::Function>::value ||
                  std::is_same<T, ast::callable::EntryFunction>::value,
              bool>::type = true>
core::optional<T> parse_function_internals(const Tokens& tokens, size_t& token,
                                           T&& fun) {
  auto tmp = token;

  while(auto var = parse_variable(tokens, tmp)) {
    fun.parameter.push_back(std::move(*var));
    if(!read_token(tokens, tmp, ",")) {
      break;  // we do not expect another variable
    }
  }
  expect_token(tokens, tmp, ")");

  auto fun_scope = parse_scope(tokens, tmp);
  if(!fun_scope) {
    UserSourceExc e;
    add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
    throw e;
  }
  const auto definitions = fun_scope->nodes.size();
  for(const auto& v : fun.parameter) {
    // Add the variables to the scope definitions in the right order
    auto it = fun_scope->nodes.end();
    std::advance(it, -definitions);
    ast::Define def(v.token);
    def.definition.emplace(v);
    fun_scope->nodes.emplace(it, std::move(def));
  }
  fun.scope = std::make_unique<ast::Scope>(*std::move(fun_scope));
  token = tmp;

  return fun;
}

core::optional<ast::callable::EntryFunction>
parse_entry_function(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, "main")) {
      expect_token(tokens, tmp, "(");
      auto fun = parse_function_internals(
          tokens, tmp, ast::callable::EntryFunction(tokens.at(token)));
      token = tmp;
      return fun;
    }
  } catch(ExceptionBase<UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "In the 'main' function defined here:"; });
    std::throw_with_nested(e);
  }
  return {};
}

core::optional<ast::callable::Function> parse_function(const Tokens& tokens,
                                                       size_t& token) {
  const static std::regex regex("([a-z][a-z0-9_]*)");
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, regex) && read_token(tokens, tmp, "(")) {
      if(is_keyword(tokens.at(token).token)) {
        UserSourceExc e;
        add_exception_info(tokens, token, e, [&] {
          e << "'" << tokens.at(token).token
            << "' is a keyword an may not be used as qualifier.";
        });
        throw e;
      }
      auto fun = parse_function_internals(
          tokens, tmp, ast::callable::Function(tokens.at(token)));
      token = tmp;
      return fun;
    }
  } catch(ExceptionBase<UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&tokens, &token, &e] {
      e << "In the '" << tokens.at(token).token << "' function defined here:";
    });
    std::throw_with_nested(e);
  }
  return {};
}

core::optional<ast::Define> parse_function_definition(const Tokens& tokens,
                                                      size_t& token) {
  auto tmp = token;
  if(read_token(tokens, tmp, "def")) {
    ast::Define def(tokens.at(token));

    if(auto entry_function = parse_entry_function(tokens, tmp)) {
      def.definition.emplace(std::move(*entry_function));
    } else if(auto function = parse_function(tokens, tmp)) {
      def.definition.emplace(std::move(*function));
    } else {
      UserSourceExc e;
      add_exception_info(tokens, tmp, e, [&] {
        e << "Unexpected token '" << tokens.at(tmp).token << '\'';
      });
      throw e;
    }
    token = tmp;
    return def;
  }
  return {};
}
core::optional<ast::Define> parse_variable_definition(const Tokens& tokens,
                                                      size_t& token) {
  auto tmp = token;
  try {
    if(read_token(tokens, tmp, "var")) {
      ast::Define def(tokens.at(token));

      if(auto variable = parse_variable(tokens, tmp)) {
        def.definition.emplace(std::move(*variable));
      } else {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e, [&] {
          e << "Unexpected token '" << tokens.at(tmp).token << '\'';
        });
        throw e;
      }
      token = tmp;
      return def;
    }
  } catch(ExceptionBase<UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&tokens, &token, &e] {
      e << "At the '" << tokens.at(token).token << "' variable defined here:";
    });
    std::throw_with_nested(e);
  }
  return {};
}

core::optional<ast::callable::Callable> parse_callable(const Tokens& tokens,
                                                       size_t& token);

core::optional<std::pair<ast::Variable, ast::ValueProducer>>
parse_callable_parameter(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  if(auto fun_var = parse_variable(tokens, tmp)) {
    if(!read_token(tokens, tmp, ":")) {
      UserSourceExc e;
      add_exception_info(tokens, tmp, e, [&] {
        e << "Expected a ':' after '" << tokens.at(tmp - 1).token
          << "' followed by an expression as value.";
      });
      throw e;
    }
    std::pair<ast::Variable, ast::ValueProducer> ret;

    if(auto exe = parse_callable(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*exe));
    } else if(auto lit_bool = parse_literal_bool(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*lit_bool));
    } else if(auto lit_int = parse_literal_int(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*lit_int));
    } else if(auto lit_double = parse_literal_double(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*lit_double));
    } else if(auto lit_string = parse_literal_string(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*lit_string));
    } else if(auto var = parse_variable(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*var));
    } else if(auto con = parse_condition(tokens, tmp)) {
      ret = std::make_pair(std::move(*fun_var), std::move(*con));
    } else {
      UserSourceExc e;
      add_exception_info(tokens, tmp, e, [&] {
        e << "Expected an expression, but found this unexpected token '"
          << tokens.at(tmp).token << '\'';
      });
      throw e;
    }

    token = tmp;
    return ret;
  }
  return {};
}

core::optional<ast::callable::Callable> parse_callable(const Tokens& tokens,
                                                       size_t& token) {
  const static std::regex regex("([a-z][a-z0-9_]*)");

  auto tmp = token;
  try {
    if(read_token(tokens, tmp, regex) && read_token(tokens, tmp, "(")) {
      if(tokens.at(token).column + tokens.at(token).token.length() !=
         tokens.at(token + 1).column) {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e, [&] {
          e << "There my not be any space between the function identifier "
               "and parentheses.";
        });
        throw e;
      }
      ast::callable::Callable call(tokens.at(token));

      while(tmp < tokens.size()) {
        if(auto param = parse_callable_parameter(tokens, tmp)) {
          call.parameter.emplace_back(std::move(*param));

          if(!read_token(tokens, tmp, ",")) {
            break;  // we do not expect another variable
          }
        } else {
          break;
        }
      }
      expect_token(tokens, tmp, ")");
      token = tmp;
      return call;
    }
  } catch(ExceptionBase<UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&tokens, &token, &e] {
      e << "In the function call '" << tokens.at(token).token
        << "' defined here:";
    });
    std::throw_with_nested(e);
  }
  return {};
}

core::optional<ast::Return> parse_return(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, "return")) {
      ast::Return ret(tokens.at(token));

      if(auto exe = parse_callable(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*exe));
      } else if(auto lit_bool = parse_literal_bool(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*lit_bool));
      } else if(auto lit_int = parse_literal_int(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*lit_int));
      } else if(auto lit_double = parse_literal_double(tokens, tmp)) {
        ret.output =
            std::make_unique<ast::ValueProducer>(std::move(*lit_double));
      } else if(auto lit_string = parse_literal_string(tokens, tmp)) {
        ret.output =
            std::make_unique<ast::ValueProducer>(std::move(*lit_string));
      } else if(auto var = parse_variable(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*var));
      } else if(auto con = parse_condition(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*con));
      } else {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e, [&] {
          e << "Unexpected token '" << tokens.at(tmp).token << '\'';
        });
        throw e;
      }

      token = tmp;
      return ret;
    }
  } catch(ExceptionBase<UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "At the return statement defined here:"; });
    std::throw_with_nested(e);
  }
  return {};
}

core::optional<ast::Break> parse_break(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, "break")) {
      ast::Break ret(tokens.at(token));

      token = tmp;
      return ret;
    }
  } catch(ExceptionBase<UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "At the break statement defined here:"; });
    std::throw_with_nested(e);
  }
  return {};
}

ast::ValueProducer node_to_value(ast::Scope::Node& node) {
  ast::ValueProducer value;
  node.match(
      [&value](ast::Variable& e) { value = std::move(e); },
      [&value](ast::callable::Callable& e) { value = std::move(e); },
      [&value](ast::UnaryOperator& e) { value = std::move(e); },
      [&value](ast::BinaryOperator& e) { value = std::move(e); },
      [&value](ast::Literal<ast::Literals::BOOL>& e) { value = std::move(e); },
      [&value](ast::Literal<ast::Literals::INT>& e) { value = std::move(e); },
      [&value](ast::Literal<ast::Literals::DOUBLE>& e) {
        value = std::move(e);
      },
      [&value](ast::Literal<ast::Literals::STRING>& e) {
        value = std::move(e);
      },
      [&value](ast::Break&) {
        ConversionExc e(__FILE__, __LINE__, "Bad conversion");
        e << "Break is not convertible to a ValueProducer.";
        throw e;
      },
      [&value](ast::Define&) {
        ConversionExc e(__FILE__, __LINE__, "Bad conversion");
        e << "Define is not convertible to a ValueProducer.";
        throw e;
      },
      [&value](ast::callable::EntryFunction&) {
        ConversionExc e(__FILE__, __LINE__, "Bad conversion");
        e << "EntryFunction is not convertible to a ValueProducer.";
        throw e;
      },
      [&value](ast::callable::Function&) {
        ConversionExc e(__FILE__, __LINE__, "Bad conversion");
        e << "Function is not convertible to a ValueProducer.";
        throw e;
      },
      [&value](ast::Return&) {
        ConversionExc e(__FILE__, __LINE__, "Bad conversion");
        e << "Return is not convertible to a ValueProducer.";
        throw e;
      },
      [&value](ast::Scope&) {
        ConversionExc e(__FILE__, __LINE__, "Bad conversion");
        e << "Scope is not convertible to a ValueProducer.";
        throw e;
      },
      [&value](ast::logic::If&) {
        ConversionExc e(__FILE__, __LINE__, "Bad conversion");
        e << "If is not convertible to a ValueProducer.";
        throw e;
      },
      [&value](ast::loop::While&) {
        ConversionExc e(__FILE__, __LINE__, "Bad conversion");
        e << "While is not convertible to a ValueProducer.";
        throw e;
      },
      [&value](ast::loop::DoWhile&) {
        ConversionExc e(__FILE__, __LINE__, "Bad conversion");
        e << "DoWhile is not convertible to a ValueProducer.";
        throw e;
      },
      [&value](ast::loop::For&) {
        ConversionExc e(__FILE__, __LINE__, "Bad conversion");
        e << "For is not convertible to a ValueProducer.";
        throw e;
      });
  return value;
}

ast::Scope::Node value_to_node(ast::ValueProducer& producer) {
  ast::Scope::Node node;
  producer.value.match(
      [&node](ast::Variable& e) { node = std::move(e); },
      [&node](ast::callable::Callable& e) { node = std::move(e); },
      [&node](ast::UnaryOperator& e) { node = std::move(e); },
      [&node](ast::BinaryOperator& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::BOOL>& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::INT>& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::DOUBLE>& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::STRING>& e) { node = std::move(e); });
  return node;
}

void assamble_operator(const std::string& file,
                       std::vector<ast::Scope::Node>& nodes, size_t& index) {
  auto next = nodes.begin();
  std::advance(next, index + 1);
  auto previous = nodes.begin();
  if(index == 0) {
    previous = nodes.end();
  } else {
    std::advance(previous, index - 1);
  }

  nodes.at(index).match(
      [&file, &nodes, &next](ast::UnaryOperator& op) {
        if(next == nodes.end()) {
          UserSourceExc e;
          add_exception_info(op.token, file, e, [&] {
            e << "Missing token for unary operator '" << op.token.token << '\'';
          });
          throw e;
        }
        op.operand = std::make_unique<ast::ValueProducer>(node_to_value(*next));
        nodes.erase(next);
      },
      [&file, &nodes, &index, &next, &previous](ast::BinaryOperator& op) {
        if(previous == nodes.end() && next == nodes.end()) {
          UserSourceExc e;
          add_exception_info(op.token, file, e, [&] {
            e << "Missing left and right hand token for binary operator '"
              << op.token.token << '\'';
          });
          throw e;
        }
        if(previous == nodes.end()) {
          UserSourceExc e;
          add_exception_info(op.token, file, e, [&] {
            e << "Missing left hand token for binary operator '"
              << op.token.token << '\'';
          });
          throw e;
        }
        if(next == nodes.end()) {
          UserSourceExc e;
          add_exception_info(op.token, file, e, [&] {
            e << "Missing right hand token for binary operator '"
              << op.token.token << '\'';
          });
          throw e;
        }
        op.left_operand =
            std::make_unique<ast::ValueProducer>(node_to_value(*previous));
        op.right_operand =
            std::make_unique<ast::ValueProducer>(node_to_value(*next));
        --index;
        nodes.erase(next);
        // We changed the vector - get new, VALID iterator
        previous = nodes.begin();
        std::advance(previous, index);  // we decremented index already
        nodes.erase(previous);
      },
      [](ast::Break&) {},                            //
      [](ast::Variable&) {},                         //
      [](ast::Define&) {},                           //
      [](ast::callable::EntryFunction&) {},          //
      [](ast::callable::Callable&) {},               //
      [](ast::callable::Function&) {},               //
      [](ast::Return&) {},                           //
      [](ast::Scope&) {},                            //
      [](ast::logic::If&) {},                        //
      [](ast::loop::While&) {},                      //
      [](ast::loop::DoWhile&) {},                    //
      [](ast::loop::For&) {},                        //
      [](ast::Literal<ast::Literals::BOOL>&) {},     //
      [](ast::Literal<ast::Literals::INT>&) {},      //
      [](ast::Literal<ast::Literals::DOUBLE>&) {},   //
      [](ast::Literal<ast::Literals::STRING>&) {});  //
}

void assamble_operators(const std::string& file,
                        std::vector<ast::Scope::Node>& nodes,
                        const ast::UnaryOperation operaton) {
  for(size_t i = 0; i < nodes.size(); ++i) {
    nodes.at(i).match(
        [&file, &nodes, &i, operaton](ast::UnaryOperator& op) {
          if(op.operation == ast::UnaryOperation::NONE) {
            OperatorExc e(__FILE__, __LINE__, "Missing operator");
            e << "1There was no type specified for the operator:" << op;
            throw e;
          } else if(op.operation == operaton && !op.operand) {
            assamble_operator(file, nodes, i);
          }
        },
        [](ast::Break&) {},                            //
        [](ast::Variable&) {},                         //
        [](ast::Define&) {},                           //
        [](ast::callable::EntryFunction&) {},          //
        [](ast::callable::Callable&) {},               //
        [](ast::callable::Function&) {},               //
        [](ast::Return&) {},                           //
        [](ast::Scope&) {},                            //
        [](ast::BinaryOperator&) {},                   //
        [](ast::logic::If&) {},                        //
        [](ast::loop::While&) {},                      //
        [](ast::loop::DoWhile&) {},                    //
        [](ast::loop::For&) {},                        //
        [](ast::Literal<ast::Literals::BOOL>&) {},     //
        [](ast::Literal<ast::Literals::INT>&) {},      //
        [](ast::Literal<ast::Literals::DOUBLE>&) {},   //
        [](ast::Literal<ast::Literals::STRING>&) {});  //
  }
}

void assamble_operators(const std::string& file,
                        std::vector<ast::Scope::Node>& nodes,
                        const ast::BinaryOperation operaton) {
  for(size_t i = 0; i < nodes.size(); ++i) {
    nodes.at(i).match(
        [&file, &nodes, &i, operaton](ast::BinaryOperator& op) {
          if(op.operation == ast::BinaryOperation::NONE) {
            OperatorExc e(__FILE__, __LINE__, "Missing operator");
            e << "2There was no type specified for the operator:" << op;
            throw e;
          } else if(op.operation == operaton && !op.left_operand &&
                    !op.right_operand) {
            assamble_operator(file, nodes, i);
          }
        },
        [](ast::Break&) {},                            //
        [](ast::Variable&) {},                         //
        [](ast::Define&) {},                           //
        [](ast::callable::EntryFunction&) {},          //
        [](ast::callable::Callable&) {},               //
        [](ast::callable::Function&) {},               //
        [](ast::Return&) {},                           //
        [](ast::Scope&) {},                            //
        [](ast::UnaryOperator&) {},                    //
        [](ast::logic::If&) {},                        //
        [](ast::loop::While&) {},                      //
        [](ast::loop::DoWhile&) {},                    //
        [](ast::loop::For&) {},                        //
        [](ast::Literal<ast::Literals::BOOL>&) {},     //
        [](ast::Literal<ast::Literals::INT>&) {},      //
        [](ast::Literal<ast::Literals::DOUBLE>&) {},   //
        [](ast::Literal<ast::Literals::STRING>&) {});  //
  }
}

void assamble_operator(const std::string& file,
                       std::vector<ast::Scope::Node>& nodes) {
  assamble_operators(file, nodes, ast::UnaryOperation::NOT);

  assamble_operators(file, nodes, ast::BinaryOperation::DIVIDE);
  assamble_operators(file, nodes, ast::BinaryOperation::MULTIPLY);
  assamble_operators(file, nodes, ast::BinaryOperation::MODULO);
  assamble_operators(file, nodes, ast::BinaryOperation::ADD);
  assamble_operators(file, nodes, ast::BinaryOperation::SUBTRACT);

  assamble_operators(file, nodes, ast::BinaryOperation::SMALLER);
  assamble_operators(file, nodes, ast::BinaryOperation::SMALLER_EQUAL);
  assamble_operators(file, nodes, ast::BinaryOperation::GREATER);
  assamble_operators(file, nodes, ast::BinaryOperation::GREATER_EQUAL);
  assamble_operators(file, nodes, ast::BinaryOperation::EQUAL);
  assamble_operators(file, nodes, ast::BinaryOperation::NOT_EQUAL);
  assamble_operators(file, nodes, ast::BinaryOperation::AND);
  assamble_operators(file, nodes, ast::BinaryOperation::OR);

  assamble_operators(file, nodes, ast::BinaryOperation::ASSIGNMENT);

  if(nodes.size() > 1) {
    Token token = node_to_token(nodes.at(1));

    UserSourceExc e;
    add_exception_info(token, file, e, [&] {
      e << "Unexpected token '" << token.token << '\'';
    });
    throw e;
  }
}

core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>>
parse_operator_internals(const Tokens& tokens, size_t& token) {
  auto tmp = token;
  core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>> ret;

  if(read_token(tokens, tmp, "!")) {
    ast::UnaryOperator op(tokens.at(token));
    op.operation = ast::UnaryOperation::NOT;
    ret.emplace(std::move(op));
  } else {
    ast::BinaryOperator op(tokens.at(token));

    if(read_token(tokens, tmp, "/")) {
      op.operation = ast::BinaryOperation::DIVIDE;
    } else if(read_token(tokens, tmp, "*")) {
      op.operation = ast::BinaryOperation::MULTIPLY;
    } else if(read_token(tokens, tmp, "%")) {
      op.operation = ast::BinaryOperation::MODULO;
    } else if(read_token(tokens, tmp, "+")) {
      op.operation = ast::BinaryOperation::ADD;
    } else if(read_token(tokens, tmp, "-")) {
      op.operation = ast::BinaryOperation::SUBTRACT;
    } else if(read_token(tokens, tmp, "<")) {
      op.operation = ast::BinaryOperation::SMALLER;
    } else if(read_token(tokens, tmp, "<=")) {
      op.operation = ast::BinaryOperation::SMALLER_EQUAL;
    } else if(read_token(tokens, tmp, ">")) {
      op.operation = ast::BinaryOperation::GREATER;
    } else if(read_token(tokens, tmp, ">=")) {
      op.operation = ast::BinaryOperation::GREATER_EQUAL;
    } else if(read_token(tokens, tmp, "==")) {
      op.operation = ast::BinaryOperation::EQUAL;
    } else if(read_token(tokens, tmp, "!=")) {
      op.operation = ast::BinaryOperation::NOT_EQUAL;
    } else if(read_token(tokens, tmp, "&&")) {
      op.operation = ast::BinaryOperation::AND;
    } else if(read_token(tokens, tmp, "||")) {
      op.operation = ast::BinaryOperation::OR;
    } else if(read_token(tokens, tmp, "=")) {
      op.operation = ast::BinaryOperation::ASSIGNMENT;
    }
    if(op.operation != ast::BinaryOperation::NONE) {
      ret.emplace(std::move(op));
    }
  }
  if(ret) {
    token = tmp;
  }
  return ret;
}

core::optional<ast::Variable> extract_var_def(ast::Scope::Node& node) {
  core::optional<ast::Variable> ret;

  node.match(
      [&ret](const ast::Define& def) {
        if(def.definition) {
          def.definition->match([&ret](const ast::Variable& var) { ret = var; },
                                [](const ast::callable::Function&) {},
                                [](const ast::callable::EntryFunction&) {});
        }
      },                                                  //
      [](const ast::Break&) {},                           //
      [](const ast::Variable&) {},                        //
      [](const ast::callable::EntryFunction&) {},         //
      [](const ast::callable::Callable&) {},              //
      [](const ast::callable::Function&) {},              //
      [](const ast::Return&) {},                          //
      [](const ast::Scope&) {},                           //
      [](const ast::UnaryOperator&) {},                   //
      [](const ast::BinaryOperator&) {},                  //
      [](const ast::logic::If&) {},                       //
      [](const ast::loop::While&) {},                     //
      [](const ast::loop::DoWhile&) {},                   //
      [](const ast::loop::For&) {},                       //
      [](const ast::Literal<ast::Literals::BOOL>&) {},    //
      [](const ast::Literal<ast::Literals::INT>&) {},     //
      [](const ast::Literal<ast::Literals::DOUBLE>&) {},  //
      [](const ast::Literal<ast::Literals::STRING>&) {});
  return ret;
}

core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>>
parse_operator(const Tokens& tokens, size_t& token,
               std::vector<ast::Scope::Node>& nodes) {
  auto tmp = token;
  core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>> ret;

  try {
    std::vector<ast::Scope::Node> workplace;

    if(auto op = parse_operator_internals(tokens, tmp)) {
      op->match(
          [&workplace](ast::UnaryOperator& op) {
            if(op.operation == ast::UnaryOperation::NONE) {
              OperatorExc e(__FILE__, __LINE__, "Missing operator");
              e << "3There was no type specified for the operator:" << op;
              throw e;
            }
            workplace.push_back(std::move(op));
          },
          [&nodes, &workplace](ast::BinaryOperator& op) {
            if(op.operation == ast::BinaryOperation::NONE) {
              OperatorExc e(__FILE__, __LINE__, "Missing operator");
              e << "4There was no type specified for the operator:" << op;
              throw e;
            }
            if(nodes.size() > 0) {
              if(auto var = extract_var_def(nodes.back())) {
                workplace.push_back(std::move(*var));
              } else {
                workplace.push_back(std::move(nodes.back()));
                nodes.erase(nodes.end() - 1);
              }
            }
            workplace.push_back(std::move(op));
          });

      while(tmp < tokens.size()) {
        if(auto exe = parse_callable(tokens, tmp)) {
          workplace.push_back(std::move(*exe));
        } else if(auto lit_bool = parse_literal_bool(tokens, tmp)) {
          workplace.push_back(std::move(*lit_bool));
        } else if(auto lit_int = parse_literal_int(tokens, tmp)) {
          workplace.push_back(std::move(*lit_int));
        } else if(auto lit_double = parse_literal_double(tokens, tmp)) {
          workplace.push_back(std::move(*lit_double));
        } else if(auto lit_string = parse_literal_string(tokens, tmp)) {
          workplace.push_back(std::move(*lit_string));
        } else if(auto var = parse_variable(tokens, tmp)) {
          workplace.push_back(std::move(*var));
        } else if(auto op = parse_operator_internals(tokens, tmp)) {
          op->match(
              [&workplace](ast::UnaryOperator& op) {
                workplace.push_back(std::move(op));
              },
              [&workplace](ast::BinaryOperator& op) {
                workplace.push_back(std::move(op));
              });
        } else if(read_token(tokens, tmp, "(")) {
          if(auto con = parse_condition(tokens, tmp)) {
            workplace.push_back(value_to_node(*con));
            expect_token(tokens, tmp, ")");
          } else {
            UserSourceExc e;
            add_exception_info(tokens, tmp, e,
                               [&] { e << "Expected an expression."; });
            throw e;
          }
        } else {
          break;  // We are done
        }
      }

      assamble_operator(tokens.file, workplace);

      if(workplace.size() == 1) {
        workplace.front().match(
            [&ret](ast::UnaryOperator& op) {
              if(op.operation == ast::UnaryOperation::NONE) {
                OperatorExc e(__FILE__, __LINE__, "Missing operator");
                e << "5There was no type specified for the operator:" << op;
                throw e;
              }
              ret.emplace(std::move(op));
            },
            [&ret](ast::BinaryOperator& op) {
              if(op.operation == ast::BinaryOperation::NONE) {
                OperatorExc e(__FILE__, __LINE__, "Missing operator");
                e << "6There was no type specified for the operator:" << op;
                throw e;
              }
              ret.emplace(std::move(op));
            },
            [&tokens](ast::Break& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::Variable& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::Define& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::callable::EntryFunction& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::callable::Callable& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::callable::Function& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::Return& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::Scope& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::logic::If& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::loop::While& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::loop::DoWhile& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::loop::For& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::Literal<ast::Literals::BOOL>& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::Literal<ast::Literals::INT>& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::Literal<ast::Literals::DOUBLE>& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            },
            [&tokens](ast::Literal<ast::Literals::STRING>& ele) {
              UserSourceExc e;
              add_exception_info(ele.token, tokens.file, e, [&] {
                e << "Unexpected token '" << ele.token.token << '\'';
              });
              throw e;
            });
      }
    }
  } catch(ExceptionBase<UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&tokens, &token, &e] {
      e << "At the operator '" << tokens.at(token).token << "' defined here:";
    });
    std::throw_with_nested(e);
  }
  if(ret) {
    token = tmp;
  }
  return ret;
}

ast::ValueProducer
assamble_conditions(const std::string& file,
                    std::vector<ast::Scope::Node> conditions) {
  assamble_operators(file, conditions, ast::UnaryOperation::NOT);

  assamble_operators(file, conditions, ast::BinaryOperation::DIVIDE);
  assamble_operators(file, conditions, ast::BinaryOperation::MULTIPLY);
  assamble_operators(file, conditions, ast::BinaryOperation::MODULO);
  assamble_operators(file, conditions, ast::BinaryOperation::ADD);
  assamble_operators(file, conditions, ast::BinaryOperation::SUBTRACT);

  assamble_operators(file, conditions, ast::BinaryOperation::SMALLER);
  assamble_operators(file, conditions, ast::BinaryOperation::SMALLER_EQUAL);
  assamble_operators(file, conditions, ast::BinaryOperation::GREATER);
  assamble_operators(file, conditions, ast::BinaryOperation::GREATER_EQUAL);
  assamble_operators(file, conditions, ast::BinaryOperation::EQUAL);
  assamble_operators(file, conditions, ast::BinaryOperation::NOT_EQUAL);
  assamble_operators(file, conditions, ast::BinaryOperation::AND);
  assamble_operators(file, conditions, ast::BinaryOperation::OR);

  if(conditions.size() > 1) {
    Token token = node_to_token(conditions.at(1));

    UserSourceExc e;
    add_exception_info(token, file, e, [&] {
      e << "Unexpected token '" << token.token << '\'';
    });
    throw e;
  }
  return node_to_value(conditions.front());
}

core::optional<ast::ValueProducer> parse_condition(const Tokens& tokens,
                                                   size_t& token) {
  std::vector<ast::Scope::Node> conditions;
  auto tmp = token;

  while(tmp < tokens.size()) {
    if(auto exe = parse_callable(tokens, tmp)) {
      conditions.push_back(std::move(*exe));
    } else if(auto lit_bool = parse_literal_bool(tokens, tmp)) {
      conditions.push_back(std::move(*lit_bool));
    } else if(auto lit_int = parse_literal_int(tokens, tmp)) {
      conditions.push_back(std::move(*lit_int));
    } else if(auto lit_double = parse_literal_double(tokens, tmp)) {
      conditions.push_back(std::move(*lit_double));
    } else if(auto lit_string = parse_literal_string(tokens, tmp)) {
      conditions.push_back(std::move(*lit_string));
    } else if(auto var = parse_variable(tokens, tmp)) {
      conditions.push_back(std::move(*var));
    } else if(auto op = parse_operator(tokens, tmp, conditions)) {
      op->match(
          [&conditions](ast::UnaryOperator& op) {
            conditions.push_back(std::move(op));
          },
          [&conditions](ast::BinaryOperator& op) {
            conditions.push_back(std::move(op));
          });
    } else if(read_token(tokens, tmp, "(")) {
      if(auto condition = parse_condition(tokens, tmp)) {
        conditions.push_back(value_to_node(*condition));
        expect_token(tokens, tmp, ")");
      } else {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e,
                           [&] { e << "Expected an expression."; });
        throw e;
      }
    } else {
      break;  // We are done
    }
  }
  if(!conditions.empty()) {
    auto condition = assamble_conditions(tokens.file, std::move(conditions));
    token = tmp;
    return condition;
  } else {
    return {};
  }
}

core::optional<ast::logic::If> parse_if(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, "if")) {
      ast::logic::If iff(tokens.at(token));

      expect_token(tokens, tmp, "(");
      auto condition = parse_condition(tokens, tmp);
      if(!condition) {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e,
                           [&] { e << "Expected an expression."; });
        throw e;
      }
      iff.condition =
          std::make_unique<ast::ValueProducer>(std::move(*condition));
      expect_token(tokens, tmp, ")");

      auto true_scope = parse_scope(tokens, tmp);
      if(!true_scope) {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
        throw e;
      }
      iff.true_scope = std::make_unique<ast::Scope>(std::move(*true_scope));

      auto tok_else = tmp;
      if(tmp < tokens.size() && read_token(tokens, tmp, "else")) {
        try {
          auto false_scope = parse_scope(tokens, tmp);
          if(!false_scope) {
            UserSourceExc e;
            add_exception_info(tokens, tmp, e,
                               [&] { e << "Expected a scope."; });
            throw e;
          }
          iff.false_scope =
              std::make_unique<ast::Scope>(std::move(*false_scope));
        } catch(ExceptionBase<UserE>&) {
          UserTailExc e;
          add_exception_info(tokens, tok_else, e,
                             [&e] { e << "In the else part defined here:"; });
          std::throw_with_nested(e);
        }
      }
      token = tmp;
      return iff;
    }
  } catch(ExceptionBase<UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "In the if defined here:"; });
    std::throw_with_nested(e);
  }
  return {};
}

core::optional<ast::loop::While> parse_while(const Tokens& tokens,
                                             size_t& token) {
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, "while")) {
      ast::loop::While w(tokens.at(token));

      expect_token(tokens, tmp, "(");
      auto condition = parse_condition(tokens, tmp);
      if(!condition) {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e,
                           [&] { e << "Expected a condition."; });
        throw e;
      }
      w.condition = std::make_unique<ast::ValueProducer>(std::move(*condition));
      expect_token(tokens, tmp, ")");

      auto scope = parse_scope(tokens, tmp);
      if(!scope) {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
        throw e;
      }
      w.scope = std::make_unique<ast::Scope>(std::move(*scope));

      token = tmp;
      return w;
    }
  } catch(ExceptionBase<UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "In the while defined here:"; });
    std::throw_with_nested(e);
  }
  return {};
}

enum class Statement { TERMINAL, NONTERMINAL };

core::optional<ast::Scope::Node>
parse_scope_internals(const Tokens& tokens, size_t& token,
                      std::vector<ast::Scope::Node>& nodes,
                      Statement& last_statement) {
  ast::Scope::Node node;

  if(auto br = parse_break(tokens, token)) {
    last_statement = Statement::NONTERMINAL;
    node = std::move(*br);
  } else if(auto def = parse_function_definition(tokens, token)) {
    last_statement = Statement::TERMINAL;
    node = std::move(*def);
  } else if(auto var_def = parse_variable_definition(tokens, token)) {
    auto tmp = token;
    if(read_token(tokens, tmp, "=")) {
      last_statement = Statement::TERMINAL;
    } else {
      last_statement = Statement::NONTERMINAL;
    }
    node = std::move(*var_def);
  } else if(auto iff = parse_if(tokens, token)) {
    last_statement = Statement::TERMINAL;
    node = std::move(*iff);
  } else if(auto whi = parse_while(tokens, token)) {
    last_statement = Statement::TERMINAL;
    node = std::move(*whi);
  } else if(auto call = parse_callable(tokens, token)) {
    last_statement = Statement::NONTERMINAL;
    node = std::move(*call);
  } else if(auto ret = parse_return(tokens, token)) {
    last_statement = Statement::NONTERMINAL;
    node = std::move(*ret);
  } else if(auto lit_bool = parse_literal_bool(tokens, token)) {
    last_statement = Statement::NONTERMINAL;
    node = std::move(*lit_bool);
  } else if(auto lit_int = parse_literal_int(tokens, token)) {
    last_statement = Statement::NONTERMINAL;
    node = std::move(*lit_int);
  } else if(auto lit_double = parse_literal_double(tokens, token)) {
    last_statement = Statement::NONTERMINAL;
    node = std::move(*lit_double);
  } else if(auto lit_string = parse_literal_string(tokens, token)) {
    last_statement = Statement::NONTERMINAL;
    node = std::move(*lit_string);
  } else if(auto op = parse_operator(tokens, token, nodes)) {
    last_statement = Statement::NONTERMINAL;
    op->match([&node](ast::UnaryOperator& op) { node = std::move(op); },
              [&node](ast::BinaryOperator& op) { node = std::move(op); });
  } else if(auto var = parse_variable(tokens, token)) {
    auto tmp = token;
    if(read_token(tokens, tmp, "=")) {
      last_statement = Statement::TERMINAL;
    } else {
      last_statement = Statement::NONTERMINAL;
    }
    node = std::move(*var);
  } else if(auto scope = parse_scope(tokens, token)) {
    last_statement = Statement::TERMINAL;
    node = std::move(*scope);
  } else if(read_token(tokens, token, "(")) {
    if(auto condition = parse_condition(tokens, token)) {
      node = value_to_node(*condition);
      expect_token(tokens, token, ")");
    } else {
      UserSourceExc e;
      add_exception_info(tokens, token, e,
                         [&] { e << "Expected an expression."; });
      throw e;
    }
  } else {
    return {};
  }
  return node;
}

void parse_scope_internals(const Tokens& tokens, size_t& token,
                           ast::Scope& scope) {
  Statement last_statement = Statement::TERMINAL;

  while(token < tokens.size()) {
    if(read_token(tokens, token, ";")) {
      last_statement = Statement::TERMINAL;
    } else if(last_statement == Statement::NONTERMINAL) {
      UserSourceExc e;
      add_exception_info(tokens, token, e, [&] { e << "Expected a ';'"; });
      throw e;
    } else if(auto node = parse_scope_internals(tokens, token, scope.nodes,
                                                last_statement)) {
      scope.nodes.push_back(std::move(*node));
    } else {
      auto tmp = token;  // No advance - that is done by the scope
      if(read_token(tokens, tmp, "}")) {
        break;  // done with the scope
      } else {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e, [&] {
          e << "Unexpected token '" << tokens.at(tmp).token << '\'';
        });
        throw e;
      }
    }
  }
  if(last_statement == Statement::NONTERMINAL) {
    UserSourceExc e;
    add_exception_info(tokens, token, e, [&] { e << "Expected a ';'"; });
    throw e;
  }
}
}

ast::Scope parse(std::string macro, std::string file_name) {
  Tokens tokens = {tokenizer::tokenize(macro), std::move(file_name)};
  auto root = ast::Scope(Token(0, 0, ""));

  for(size_t i = 0; i < tokens.size(); ++i) {
    parse_scope_internals(tokens, i, root);
  }

  // TODO check that no break is in a scope that is not contained by a loop
  // (functions reset the counter)
  // TODO check that no ast elements follow a break
  // TODO check that no ast elements follow a return
  // TODO check that a main function is given

  return root;
}
}
}
}
