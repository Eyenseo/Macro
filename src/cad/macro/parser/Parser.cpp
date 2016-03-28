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
using ConversionExc = Exc<Parser::InternalE, Parser::InternalE::BAD_CONVERSION>;
using OperatorExc = Exc<Parser::InternalE, Parser::InternalE::MISSING_OPERATOR>;
using StringEndExc = Exc<Parser::InternalE, Parser::InternalE::STRING_END>;
using UserSourceExc = Exc<Parser::UserE, Parser::UserE::SOURCE>;
using UserTailExc = Exc<Parser::UserE, Parser::UserE::TAIL>;

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
                        ExceptionBase<Parser::UserE>& e, FUN fun) {
  e << file << ':' << token.line << ':' << token.column << ": ";
  fun();
  if(token.source_line) {
    e << '\n' << *token.source_line << '\n'
      << std::string(token.column - 1, ' ') << "^";
  }
}
template <typename FUN>
void add_exception_info(const Tokens& tokens, const size_t token,
                        ExceptionBase<Parser::UserE>& e, FUN fun) {
  add_exception_info(tokens.at(token), tokens.file, e, fun);
}

void expect_token(const Tokens& tokens, size_t& token,
                  const char* const token_literal) {
  if(tokens.at(token).token != token_literal) {
    UserSourceExc e;
    add_exception_info(tokens, token, e,
                       [&] { e << "Missing '" << token_literal << "'"; });
    throw e;
  }
  auto tmp = token + 1;
  if(token + 1 > tokens.size()) {
    StringEndExc e(__FILE__, __LINE__, "Unexpected string end");
    e << "The string ended unexpectedly " << tokens.at(token);
    throw e;
  }
  token = tmp;
}

bool read_token(const Tokens& tokens, size_t& token,
                const char* const token_literal) {
  if(tokens.at(token).token != token_literal) {
    return false;
  }
  auto tmp = token + 1;
  if(tmp > tokens.size()) {
    StringEndExc e(__FILE__, __LINE__, "Unexpected string end");
    e << "The string ended unexpectedly " << tokens.at(token).token;
    throw e;
  }
  token = tmp;
  return true;
}

bool read_token(const Tokens& tokens, size_t& token,
                const std::regex& token_regex) {
  std::smatch match;
  if(!std::regex_match(tokens.at(token).token, match, token_regex)) {
    return false;
  }
  auto tmp = token + 1;
  if(tmp > tokens.size()) {
    StringEndExc e(__FILE__, __LINE__, "Unexpected string end");
    e << "The string ended unexpectedly " << tokens.at(token).token;
    throw e;
  }
  token = tmp;
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
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
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
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
    std::throw_with_nested(e);
  }
  return {};
}

core::optional<ast::Define> parse_function_definition(const Tokens& tokens,
                                                      size_t& token) {
  auto tmp = token;
  try {
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
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
    std::throw_with_nested(e);
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
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
    std::throw_with_nested(e);
  }
  return {};
}

core::optional<ast::callable::Callable> parse_callable(const Tokens& tokens,
                                                       size_t& token);

core::optional<std::pair<ast::Variable, ast::ValueProducer>>
parse_callable_parameter(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  try {
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
          e << "Unexpected token '" << tokens.at(tmp).token << '\'';
        });
        throw e;
      }

      token = tmp;
      return ret;
    }
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
    std::throw_with_nested(e);
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
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
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
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
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
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
    std::throw_with_nested(e);
  }
  return {};
}

core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>>
parse_operator(const Tokens& tokens, size_t& token) {
  auto tmp = token;
  core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>> ret;

  try {
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
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
    std::throw_with_nested(e);
  }
  if(ret) {
    token = tmp;
  }
  return ret;
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
                       std::vector<ast::Scope::Node>& nodes, const size_t start,
                       size_t& index) {
  auto next = nodes.begin();
  std::advance(next, index + 1);
  auto previous = nodes.begin();
  if(index == 0 || start == index) {
    previous = nodes.end();
  } else {
    std::advance(previous, index - 1);
  }

  nodes.at(index).match(
      [&file, &nodes, &next](ast::UnaryOperator& op) {
        if(next == nodes.end()) {
          UserSourceExc e;
          add_exception_info(op.token, file, e, [&] {
            e << "Missing token for unary operator '" << op.token << '\'';
          });
          throw e;
        }
        op.operand = std::make_unique<ast::ValueProducer>(node_to_value(*next));
        nodes.erase(next);
      },
      [&file, &nodes, &index, &next, &previous](ast::BinaryOperator& op) {
        if(next == nodes.end() || previous == nodes.end()) {
          UserSourceExc e;
          add_exception_info(op.token, file, e, [&] {
            e << "Missing token for binary operator '" << op.token << '\'';
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
                        const size_t start,
                        const ast::UnaryOperation operaton) {
  for(size_t i = start; i < nodes.size(); ++i) {
    nodes.at(i).match(
        [&file, &nodes, start, &i, operaton](ast::UnaryOperator& op) {
          if(op.operation == ast::UnaryOperation::NONE) {
            OperatorExc e(__FILE__, __LINE__, "Missing operator");
            e << "There was no type specified for the operator:" << op;
            throw e;
          } else if(op.operation == operaton) {
            assamble_operator(file, nodes, start, i);
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
                        const size_t start,
                        const ast::BinaryOperation operaton) {
  for(size_t i = start; i < nodes.size(); ++i) {
    nodes.at(i).match(
        [&file, &nodes, start, &i, operaton](ast::BinaryOperator& op) {
          if(op.operation == ast::BinaryOperation::NONE) {
            OperatorExc e(__FILE__, __LINE__, "Missing operator");
            e << "There was no type specified for the operator:" << op;
            throw e;
          } else if(op.operation == operaton) {
            assamble_operator(file, nodes, start, i);
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

ast::ValueProducer
assamble_conditions(const std::string& file,
                    std::vector<ast::Scope::Node> conditions) {
  assamble_operators(file, conditions, 0, ast::UnaryOperation::NOT);

  assamble_operators(file, conditions, 0, ast::BinaryOperation::DIVIDE);
  assamble_operators(file, conditions, 0, ast::BinaryOperation::MULTIPLY);
  assamble_operators(file, conditions, 0, ast::BinaryOperation::MODULO);
  assamble_operators(file, conditions, 0, ast::BinaryOperation::ADD);
  assamble_operators(file, conditions, 0, ast::BinaryOperation::SUBTRACT);

  assamble_operators(file, conditions, 0, ast::BinaryOperation::SMALLER);
  assamble_operators(file, conditions, 0, ast::BinaryOperation::SMALLER_EQUAL);
  assamble_operators(file, conditions, 0, ast::BinaryOperation::GREATER);
  assamble_operators(file, conditions, 0, ast::BinaryOperation::GREATER_EQUAL);
  assamble_operators(file, conditions, 0, ast::BinaryOperation::EQUAL);
  assamble_operators(file, conditions, 0, ast::BinaryOperation::NOT_EQUAL);
  assamble_operators(file, conditions, 0, ast::BinaryOperation::AND);
  assamble_operators(file, conditions, 0, ast::BinaryOperation::OR);

  if(conditions.size() != 1) {
    Token token = node_to_token(conditions.at(2));

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

  try {
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
      } else if(auto op = parse_operator(tokens, tmp)) {
        op->match(
            [&conditions](ast::UnaryOperator& op) {
              conditions.push_back(std::move(op));
            },
            [&conditions](ast::BinaryOperator& op) {
              conditions.push_back(std::move(op));
            });
      } else if(read_token(tokens, tmp, "(")) {
        auto condition = parse_condition(tokens, tmp);
        if(!condition) {
          UserSourceExc e;
          add_exception_info(tokens, tmp, e,
                             [&] { e << "Expected a expression."; });
          throw e;
        }
        conditions.push_back(value_to_node(*condition));
        expect_token(tokens, tmp, ")");
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
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
    std::throw_with_nested(e);
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
                           [&] { e << "Expected a expression."; });
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

      token = tmp;
      if(tmp < tokens.size() && read_token(tokens, tmp, "else")) {
        auto false_scope = parse_scope(tokens, tmp);
        if(!false_scope) {
          UserSourceExc e;
          add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
          throw e;
        }
        iff.false_scope = std::make_unique<ast::Scope>(std::move(*false_scope));
      }

      token = tmp;
      return iff;
    }
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
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
  } catch(ExceptionBase<Parser::UserE>&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&e] { e << ""; });
    std::throw_with_nested(e);
  }
  return {};
}

void two_step_define_assign(const std::string& file,
                            std::vector<ast::Scope::Node>& nodes,
                            std::vector<ast::Scope::Node>::iterator current,
                            size_t& index, ast::Define& define) {
  if(!define.definition) {
    UserSourceExc e;
    add_exception_info(define.token, file, e,
                       [&] { e << "Expected a expression."; });
    throw define;
  }
  define.definition->match(
      [&nodes, &index, &current](ast::Variable& v) {
        nodes.emplace(current, v);
        ++index;
      },
      [](ast::callable::EntryFunction&) {},  //
      [](ast::callable::Function&) {}        //
      );
}

void two_step_define_assign(const std::string& file,
                            std::vector<ast::Scope::Node>& nodes,
                            std::vector<ast::Scope::Node>::iterator previous,
                            std::vector<ast::Scope::Node>::iterator current,
                            size_t& index) {
  if(previous == nodes.end()) {
    Token token = node_to_token(*current);
    UserSourceExc e;
    add_exception_info(token, file, e, [&] { e << "Expected a expression."; });
    throw e;
  }
  previous->match(
      [&file, &nodes, &index, &current](ast::Define& define) {
        two_step_define_assign(file, nodes, current, index, define);
      },                                             //
      [](ast::Break&) {},                            //
      [](ast::Variable&) {},                         //
      [](ast::callable::EntryFunction&) {},          //
      [](ast::callable::Callable&) {},               //
      [](ast::callable::Function&) {},               //
      [](ast::Return&) {},                           //
      [](ast::Scope&) {},                            //
      [](ast::UnaryOperator&) {},                    //
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

void two_step_define_assign(const std::string& file,
                            std::vector<ast::Scope::Node>& nodes,
                            const size_t start, size_t& index) {
  auto current = nodes.begin();
  std::advance(current, index);
  auto previous = nodes.begin();
  if(index == 0 || start == index) {
    previous = nodes.end();
  } else {
    std::advance(previous, index - 1);
  }

  nodes.at(index).match(
      [&file, &nodes, &index, &current, &previous](ast::BinaryOperator&) {
        two_step_define_assign(file, nodes, previous, current, index);
      },
      [](ast::Break&) {},                            //
      [](ast::Variable&) {},                         //
      [](ast::Define&) {},                           //
      [](ast::UnaryOperator&) {},                    //
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

void two_step_define_assign(const std::string& file,
                            std::vector<ast::Scope::Node>& nodes,
                            const size_t start) {
  for(size_t i = start; i < nodes.size(); ++i) {
    nodes.at(i).match(
        [&file, &nodes, start, &i](ast::BinaryOperator& op) {
          if(op.operation == ast::BinaryOperation::NONE) {
            OperatorExc e(__FILE__, __LINE__, "Missing operator");
            e << "There was no type specified for the operator:" << op;
            throw e;
          } else if(op.operation == ast::BinaryOperation::ASSIGNMENT) {
            two_step_define_assign(file, nodes, start, i);
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

void assamble_statement(const std::string& file, ast::Scope& scope,
                        const size_t last) {
  two_step_define_assign(file, scope.nodes, last);

  assamble_operators(file, scope.nodes, last, ast::UnaryOperation::NOT);

  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::DIVIDE);
  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::MULTIPLY);
  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::MODULO);
  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::ADD);
  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::SUBTRACT);

  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::SMALLER);
  assamble_operators(file, scope.nodes, last,
                     ast::BinaryOperation::SMALLER_EQUAL);
  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::GREATER);
  assamble_operators(file, scope.nodes, last,
                     ast::BinaryOperation::GREATER_EQUAL);
  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::EQUAL);
  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::NOT_EQUAL);
  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::AND);
  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::OR);

  assamble_operators(file, scope.nodes, last, ast::BinaryOperation::ASSIGNMENT);
}

enum class Statement {
  NONE,
  BR,
  FUN_DEF,
  VAR_DEF,
  IF,
  WHILE,
  CALL,
  RET,
  LIT,
  OP,
  VAR,
  SCOPE
};

core::optional<ast::Scope::Node>
parse_scope_internals(const Tokens& tokens, size_t& token,
                      Statement& last_statement) {
  ast::Scope::Node node;

  if(auto br = parse_break(tokens, token)) {
    if(last_statement != Statement::NONE &&
       last_statement != Statement::SCOPE &&
       last_statement != Statement::FUN_DEF &&
       last_statement != Statement::WHILE && last_statement != Statement::IF) {
      UserSourceExc e;
      add_exception_info(tokens, token, e, [&] { e << "Expected a ';'"; });
      throw e;
    }
    last_statement = Statement::BR;
    node = std::move(*br);
  } else if(auto def = parse_function_definition(tokens, token)) {
    if(last_statement != Statement::NONE &&
       last_statement != Statement::SCOPE &&
       last_statement != Statement::FUN_DEF &&
       last_statement != Statement::WHILE && last_statement != Statement::IF) {
      UserSourceExc e;
      add_exception_info(tokens, token, e, [&] { e << "Expected a ';'"; });
      throw e;
    }
    last_statement = Statement::FUN_DEF;
    node = std::move(*def);
  } else if(auto def = parse_variable_definition(tokens, token)) {
    if(last_statement != Statement::NONE &&
       last_statement != Statement::SCOPE &&
       last_statement != Statement::FUN_DEF &&
       last_statement != Statement::WHILE && last_statement != Statement::IF) {
      UserSourceExc e;
      add_exception_info(tokens, token, e, [&] { e << "Expected a ';'"; });
      throw e;
    }
    last_statement = Statement::VAR_DEF;
    node = std::move(*def);
  } else if(auto iff = parse_if(tokens, token)) {
    if(last_statement != Statement::NONE &&
       last_statement != Statement::SCOPE &&
       last_statement != Statement::FUN_DEF &&
       last_statement != Statement::WHILE && last_statement != Statement::IF) {
      UserSourceExc e;
      add_exception_info(tokens, token, e, [&] { e << "Expected a ';'"; });
      throw e;
    }
    last_statement = Statement::IF;
    node = std::move(*iff);
  } else if(auto whi = parse_while(tokens, token)) {
    if(last_statement != Statement::NONE &&
       last_statement != Statement::SCOPE &&
       last_statement != Statement::FUN_DEF &&
       last_statement != Statement::WHILE && last_statement != Statement::IF) {
      UserSourceExc e;
      add_exception_info(tokens, token, e, [&] { e << "Expected a ';'"; });
      throw e;
    }
    last_statement = Statement::WHILE;
    node = std::move(*whi);
  } else if(auto exe = parse_callable(tokens, token)) {
    if(last_statement != Statement::NONE &&
       last_statement != Statement::SCOPE &&
       last_statement != Statement::FUN_DEF &&
       last_statement != Statement::WHILE && last_statement != Statement::IF &&
       last_statement != Statement::OP) {
      UserSourceExc e;
      add_exception_info(tokens, token, e, [&] { e << "Expected a ';'"; });
      throw e;
    }
    last_statement = Statement::CALL;
    node = std::move(*exe);
  } else if(auto ret = parse_return(tokens, token)) {
    if(last_statement != Statement::NONE &&
       last_statement != Statement::SCOPE &&
       last_statement != Statement::FUN_DEF &&
       last_statement != Statement::WHILE && last_statement != Statement::IF) {
      UserSourceExc e;
      add_exception_info(tokens, token, e, [&] { e << "Expected a ';'"; });
      throw e;
    }
    last_statement = Statement::RET;
    node = std::move(*ret);
  } else if(auto lit_bool = parse_literal_bool(tokens, token)) {
    last_statement = Statement::LIT;
    node = std::move(*lit_bool);
  } else if(auto lit_int = parse_literal_int(tokens, token)) {
    last_statement = Statement::LIT;
    node = std::move(*lit_int);
  } else if(auto lit_double = parse_literal_double(tokens, token)) {
    last_statement = Statement::LIT;
    node = std::move(*lit_double);
  } else if(auto lit_string = parse_literal_string(tokens, token)) {
    last_statement = Statement::LIT;
    node = std::move(*lit_string);
  } else if(auto op = parse_operator(tokens, token)) {
    last_statement = Statement::OP;
    op->match([&node](ast::UnaryOperator& op) { node = std::move(op); },
              [&node](ast::BinaryOperator& op) { node = std::move(op); });
  } else if(auto var = parse_variable(tokens, token)) {
    last_statement = Statement::VAR;
    node = std::move(*var);
  } else if(auto scope = parse_scope(tokens, token)) {
    if(last_statement != Statement::NONE &&
       last_statement != Statement::SCOPE &&
       last_statement != Statement::FUN_DEF &&
       last_statement != Statement::WHILE && last_statement != Statement::IF) {
      UserSourceExc e;
      add_exception_info(tokens, token, e, [&] { e << "Expected a ';'"; });
      throw e;
    }
    last_statement = Statement::SCOPE;
    node = std::move(*scope);
  } else {
    return {};
  }
  return node;
}

void parse_scope_internals(const Tokens& tokens, size_t& token,
                           ast::Scope& scope) {
  auto last = scope.nodes.size();
  Statement last_statement = Statement::NONE;

  while(token < tokens.size()) {
    if(read_token(tokens, token, ";")) {
      assamble_statement(tokens.file, scope, last);
      last = scope.nodes.size();
      last_statement = Statement::NONE;
    } else if(auto node =
                  parse_scope_internals(tokens, token, last_statement)) {
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
}
}

Parser::Parser() {
}

ast::Scope Parser::parse(std::string macro, std::string file_name) const {
  Tokens tokens = {tokenizer::tokenize(macro), std::move(file_name)};
  auto root = ast::Scope(Token(0, 0, ""));

  for(size_t i = 0; i < tokens.size(); ++i) {
    parse_scope_internals(tokens, i, root);
  }

  return root;
}
}
}
}
