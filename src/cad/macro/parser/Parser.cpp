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
using UserExc = ExceptionBase<UserE>;

template <typename T>
using is_Function = typename std::enable_if<
    std::is_same<T, ast::callable::Function>::value ||
        std::is_same<T, ast::callable::EntryFunction>::value,
    bool>;
template <typename T>
using is_Operator =
    typename std::enable_if<std::is_same<T, ast::UnaryOperator>::value ||
                                std::is_same<T, ast::BinaryOperator>::value,
                            bool>;

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

enum class Statement { TERMINATED, NON_TERMINATED };

static std::vector<std::string> keywords{"if",    "else",   "do",   "while",
                                         "for",   "var",    "def",  "main",
                                         "break", "return", "true", "false"};

//////////////////////////////////////////
/// Exception
//////////////////////////////////////////
Token node_to_token(const ast::Scope::Node& node);
template <typename FUN>
void add_exception_info(const Token& token, const std::string& file, UserExc& e,
                        FUN fun, const size_t arrow_position);
template <typename FUN>
void add_exception_info(const Token& token, const std::string& file, UserExc& e,
                        FUN fun);
template <typename FUN>
void add_exception_info_end(const Token& token, const std::string& file,
                            UserExc& e, FUN fun);
template <typename FUN>
void add_exception_info(const Tokens& tokens, const size_t token, UserExc& e,
                        FUN fun);
[[noreturn]] void throw_unexprected_token(const Tokens& tokens,
                                          const size_t token);
[[noreturn]] void throw_unexprected_token(const Token& token,
                                          const std::string& file);
[[noreturn]] void throw_conversion(const char* const file, const size_t line,
                                   const char* const prefix);
[[noreturn]] void throw_conversion(const char* const file, const size_t line,
                                   const char* const prefix);
void expect_no_space_between_bracket(const Tokens& tokens, const size_t token);

//////////////////////////////////////////
/// Token reading
//////////////////////////////////////////
void expect_token(const Tokens& tokens, size_t& token,
                  const char* const token_literal);

bool read_token(const Tokens& tokens, size_t& token,
                const char* const token_literal);

bool read_token(const Tokens& tokens, size_t& token,
                const std::regex& token_regex);

//////////////////////////////////////////
/// Literal parsing
//////////////////////////////////////////
core::optional<ast::Literal<ast::Literals::BOOL>>
parse_literal_bool(const Tokens& tokens, size_t& token);

core::optional<ast::Literal<ast::Literals::INT>>
parse_literal_int(const Tokens& tokens, size_t& token);

core::optional<ast::Literal<ast::Literals::DOUBLE>>
parse_literal_double(const Tokens& tokens, size_t& token);

void unescape_string(std::string& s);

core::optional<ast::Literal<ast::Literals::STRING>>
parse_literal_string(const Tokens& tokens, size_t& token);

//////////////////////////////////////////
/// Definition parsing
//////////////////////////////////////////
bool is_keyword(const std::string& token);
void expect_not_keyword(const Tokens& tokens, const size_t token);
template <typename T, typename is_Function<T>::type = false>
void parse_function_parameter(const Tokens& tokens, size_t& token, T& fun);
template <typename T, typename is_Function<T>::type = true>
core::optional<T> parse_function_internals(const Tokens& tokens, size_t& token,
                                           T&& fun);
core::optional<ast::callable::EntryFunction>
parse_entry_function(const Tokens& tokens, size_t& token);
core::optional<ast::callable::Function> parse_function(const Tokens& tokens,
                                                       size_t& token);
core::optional<ast::Define> parse_function_definition(const Tokens& tokens,
                                                      size_t& token);
core::optional<ast::Define> parse_variable_definition(const Tokens& tokens,
                                                      size_t& token);

//////////////////////////////////////////
/// Callable parsing
//////////////////////////////////////////
void expect_named_parameter(const Tokens& tokens, size_t& token);
core::optional<std::pair<ast::Variable, ast::ValueProducer>>
parse_callable_parameter(const Tokens& tokens, size_t& token);
core::optional<ast::callable::Callable> parse_callable(const Tokens& tokens,
                                                       size_t& token);

//////////////////////////////////////////
/// Scope parsing
//////////////////////////////////////////
core::optional<ast::Scope::Node>
parse_scope_internals(const Tokens& tokens, size_t& token,
                      std::vector<ast::Scope::Node>& nodes,
                      Statement& last_statement);
void parse_scope_internals(const Tokens& tokens, size_t& token,
                           ast::Scope& scope);
core::optional<ast::Scope> parse_scope(const Tokens& tokens, size_t& token);


//////////////////////////////////////////
/// Variable parsing
//////////////////////////////////////////
core::optional<ast::Variable> parse_variable(const Tokens& tokens,
                                             size_t& token);

//////////////////////////////////////////
/// Return parsing
//////////////////////////////////////////
core::optional<ast::Return> parse_return(const Tokens& tokens, size_t& token);

//////////////////////////////////////////
/// Break parsing
//////////////////////////////////////////
core::optional<ast::Break> parse_break(const Tokens& tokens, size_t& token);

//////////////////////////////////////////
/// Operator parsing
//////////////////////////////////////////
template <typename T, typename is_Operator<T>::type = false>
void expect_operator_type(const char* const file, const size_t line,
                          const T& op);
ast::ValueProducer node_to_value(ast::Scope::Node& node);
ast::Scope::Node value_to_node(ast::ValueProducer& producer);
core::variant<ast::UnaryOperator, ast::BinaryOperator>
node_to_operator(const Tokens& tokens, ast::Scope::Node& node);
core::optional<ast::Variable> extract_var_def(ast::Scope::Node& node);
void setup_operator_wokespace(
    std::vector<ast::Scope::Node>& workspace,
    std::vector<ast::Scope::Node>& nodes,
    ::core::variant<ast::UnaryOperator, ast::BinaryOperator>& op);
void expect_operatees(const Token& token, const std::string& file,
                      const std::vector<ast::Scope::Node>::iterator& previous,
                      const std::vector<ast::Scope::Node>::iterator& next,
                      const std::vector<ast::Scope::Node>::iterator& end);
void assamble_unary(const std::string& file,
                    std::vector<ast::Scope::Node>& nodes,
                    std::vector<ast::Scope::Node>::iterator& next,
                    ast::UnaryOperator& op);
void assamble_binary(const std::string& file,
                     std::vector<ast::Scope::Node>& nodes, size_t& index,
                     std::vector<ast::Scope::Node>::iterator& previous,
                     std::vector<ast::Scope::Node>::iterator& next,
                     ast::BinaryOperator& op);
void assamble_operator(const std::string& file,
                       std::vector<ast::Scope::Node>& nodes, size_t& index);
void assamble_operators(const std::string& file,
                        std::vector<ast::Scope::Node>& nodes,
                        const ast::UnaryOperation operaton);
void assamble_operators(const std::string& file,
                        std::vector<ast::Scope::Node>& nodes,
                        const ast::BinaryOperation operaton);
void assamble_operator(const std::string& file,
                       std::vector<ast::Scope::Node>& nodes);
core::optional<ast::UnaryOperator> parse_unary_operator(const Tokens& tokens,
                                                        size_t& token);
core::optional<ast::BinaryOperator> parse_binary_operator(const Tokens& tokens,
                                                          size_t& token);
core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>>
parse_operator_internals(const Tokens& tokens, size_t& token);
core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>>
parse_operator(const Tokens& tokens, size_t& token,
               std::vector<ast::Scope::Node>& nodes);

//////////////////////////////////////////
/// Condition parsing
//////////////////////////////////////////
ast::ValueProducer
assamble_conditions(const std::string& file,
                    std::vector<ast::Scope::Node> conditions);
core::optional<ast::ValueProducer> parse_condition(const Tokens& tokens,
                                                   size_t& token);

//////////////////////////////////////////
/// If parsing
//////////////////////////////////////////
void parse_if_condition(const Tokens& tokens, size_t& token,
                        ast::logic::If& iff);
void parse_true(const Tokens& tokens, size_t& token, ast::logic::If& iff);
void parse_false(const Tokens& tokens, size_t& token, ast::logic::If& iff);
core::optional<ast::logic::If> parse_if(const Tokens& tokens, size_t& token);

//////////////////////////////////////////
/// While parsing
//////////////////////////////////////////
void parse_while_condition(const Tokens& tokens, size_t& token,
                           ast::loop::While& whi);
void parse_while_scope(const Tokens& tokens, size_t& token,
                       ast::loop::While& whi);
core::optional<ast::loop::While> parse_while(const Tokens& tokens,
                                             size_t& token);

//////////////////////////////////////////
/// Implementation
//////////////////////////////////////////

//////////////////////////////////////////
/// Exception
//////////////////////////////////////////
Token node_to_token(const ast::Scope::Node& node) {
  using namespace ast;
  Token token;

  node.match([&token](const Break& e) { token = e.token; },
             [&token](const Variable& e) { token = e.token; },
             [&token](const Define& e) { token = e.token; },
             [&token](const callable::Callable& e) { token = e.token; },
             [&token](const Return& e) { token = e.token; },
             [&token](const Scope& e) { token = e.token; },
             [&token](const UnaryOperator& e) { token = e.token; },
             [&token](const BinaryOperator& e) { token = e.token; },
             [&token](const logic::If& e) { token = e.token; },
             [&token](const loop::While& e) { token = e.token; },
             [&token](const loop::DoWhile& e) { token = e.token; },
             [&token](const loop::For& e) { token = e.token; },
             [&token](const Literal<Literals::BOOL>& e) { token = e.token; },
             [&token](const Literal<Literals::INT>& e) { token = e.token; },
             [&token](const Literal<Literals::DOUBLE>& e) { token = e.token; },
             [&token](const Literal<Literals::STRING>& e) { token = e.token; });
  return token;
}

template <typename FUN>
void add_exception_info(const Token& token, const std::string& file, UserExc& e,
                        FUN fun, const size_t arrow_position) {
  e << file << ':' << token.line << ':' << token.column << ": ";
  fun();
  if(token.source_line) {
    e << '\n' << *token.source_line << '\n' << std::string(arrow_position, ' ')
      << "^";
  }
}

template <typename FUN>
void add_exception_info(const Token& token, const std::string& file, UserExc& e,
                        FUN fun) {
  add_exception_info(token, file, e, fun, token.column - 1);
}

template <typename FUN>
void add_exception_info_end(const Token& token, const std::string& file,
                            UserExc& e, FUN fun) {
  add_exception_info(token, file, e, fun,
                     token.column + token.token.size() - 1);
}

template <typename FUN>
void add_exception_info(const Tokens& tokens, const size_t token, UserExc& e,
                        FUN fun) {
  if(token == tokens.size()) {
    add_exception_info_end(tokens.at(token - 1), tokens.file, e, fun);
  } else {
    add_exception_info(tokens.at(token), tokens.file, e, fun);
  }
}

[[noreturn]] void throw_unexprected_token(const Tokens& tokens,
                                          const size_t token) {
  UserSourceExc e;
  add_exception_info(tokens, token, e, [&] {
    e << "Unexpected token '" << tokens.at(token).token << '\'';
  });
  throw e;
}
[[noreturn]] void throw_unexprected_token(const Token& token,
                                          const std::string& file) {
  UserSourceExc e;
  add_exception_info(token, file, e,
                     [&] { e << "Unexpected token '" << token.token << '\''; });
  throw e;
}

[[noreturn]] void throw_conversion(const char* const file, const size_t line,
                                   const char* const prefix) {
  ConversionExc e(file, line, "Bad conversion");
  e << prefix << " is not convertible to a ValueProducer.";
  throw e;
}

//////////////////////////////////////////
/// Token reading
//////////////////////////////////////////
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

//////////////////////////////////////////
/// Literal parsing
//////////////////////////////////////////
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

//////////////////////////////////////////
/// Definition parsing
//////////////////////////////////////////
bool is_keyword(const std::string& token) {
  for(const auto& w : keywords) {
    if(token == w) {
      return true;
    }
  }
  return false;
}

void expect_not_keyword(const Tokens& tokens, const size_t token) {
  if(is_keyword(tokens.at(token).token)) {
    UserSourceExc e;
    add_exception_info(tokens, token, e, [&] {
      e << "'" << tokens.at(token).token
        << "' is a keyword an may not be used as qualifier.";
    });
    throw e;
  }
}

template <typename T, typename is_Function<T>::type>
void parse_function_parameter(const Tokens& tokens, size_t& token, T& fun) {
  while(auto var = parse_variable(tokens, token)) {
    fun.parameter.push_back(std::move(*var));
    if(!read_token(tokens, token, ",")) {
      break;  // we do not expect another variable
    }
  }
}

template <typename T, typename is_Function<T>::type>
core::optional<T> parse_function_internals(const Tokens& tokens, size_t& token,
                                           T&& fun) {
  auto tmp = token;

  parse_function_parameter(tokens, tmp, fun);
  expect_token(tokens, tmp, ")");

  auto fun_scope = parse_scope(tokens, tmp);
  if(!fun_scope) {
    UserSourceExc e;
    add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
    throw e;
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
  } catch(UserExc&) {
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
      expect_not_keyword(tokens, token);
      expect_no_space_between_bracket(tokens, token);

      auto fun = parse_function_internals(
          tokens, tmp, ast::callable::Function(tokens.at(token)));

      token = tmp;
      return fun;
    }
  } catch(UserExc&) {
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
      def.definition = std::move(*entry_function);
    } else if(auto function = parse_function(tokens, tmp)) {
      def.definition = std::move(*function);
    } else {
      throw_unexprected_token(tokens, tmp);
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
        def.definition = std::move(*variable);
      } else {
        throw_unexprected_token(tokens, tmp);
      }
      token = tmp;
      return def;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&tokens, &token, &e] {
      e << "At the '" << tokens.at(token).token << "' variable defined here:";
    });
    std::throw_with_nested(e);
  }
  return {};
}

//////////////////////////////////////////
/// Callable parsing
//////////////////////////////////////////
void expect_named_parameter(const Tokens& tokens, size_t& token) {
  if(!read_token(tokens, token, ":")) {
    UserSourceExc e;
    add_exception_info(tokens, token, e, [&] {
      e << "Expected a ':' after '" << tokens.at(token - 1).token
        << "' followed by an expression as value.";
    });
    throw e;
  }
}

void expect_no_space_between_bracket(const Tokens& tokens, const size_t token) {
  if(tokens.at(token).column + tokens.at(token).token.length() !=
     tokens.at(token + 1).column) {
    UserSourceExc e;
    add_exception_info(tokens, token, e, [&] {
      e << "There my not be any space between the function identifier and "
           "parentheses.";
    });
    throw e;
  }
}

core::optional<std::pair<ast::Variable, ast::ValueProducer>>
parse_callable_parameter(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  if(auto fun_var = parse_variable(tokens, tmp)) {
    expect_named_parameter(tokens, tmp);
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
      expect_no_space_between_bracket(tokens, token);

      ast::callable::Callable call(tokens.at(token));

      while(tmp < tokens.size()) {
        if(auto param = parse_callable_parameter(tokens, tmp)) {
          call.parameter.emplace_back(std::move(*param));

          if(!read_token(tokens, tmp, ",")) {
            break;  // we do not expect another variable
          }
        } else {
          break;  // the exception comes from the parenthesis check
        }
      }
      expect_token(tokens, tmp, ")");
      token = tmp;
      return call;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e, [&tokens, &token, &e] {
      e << "In the function call '" << tokens.at(token).token
        << "' defined here:";
    });
    std::throw_with_nested(e);
  }
  return {};
}

//////////////////////////////////////////
/// Scope parsing
//////////////////////////////////////////
core::optional<ast::Scope::Node>
parse_scope_internals(const Tokens& tokens, size_t& token,
                      std::vector<ast::Scope::Node>& nodes,
                      Statement& last_statement) {
  ast::Scope::Node node;

  if(auto br = parse_break(tokens, token)) {
    last_statement = Statement::NON_TERMINATED;
    node = std::move(*br);
  } else if(auto def = parse_function_definition(tokens, token)) {
    last_statement = Statement::TERMINATED;
    node = std::move(*def);
  } else if(auto var_def = parse_variable_definition(tokens, token)) {
    auto tmp = token;
    if(read_token(tokens, tmp, "=")) {
      last_statement = Statement::TERMINATED;
    } else {
      last_statement = Statement::NON_TERMINATED;
    }
    node = std::move(*var_def);
  } else if(auto iff = parse_if(tokens, token)) {
    last_statement = Statement::TERMINATED;
    node = std::move(*iff);
  } else if(auto whi = parse_while(tokens, token)) {
    last_statement = Statement::TERMINATED;
    node = std::move(*whi);
  } else if(auto ret = parse_return(tokens, token)) {
    last_statement = Statement::NON_TERMINATED;
    node = std::move(*ret);
  } else if(auto call = parse_callable(tokens, token)) {
    last_statement = Statement::NON_TERMINATED;
    node = std::move(*call);
  } else if(auto lit_bool = parse_literal_bool(tokens, token)) {
    last_statement = Statement::NON_TERMINATED;
    node = std::move(*lit_bool);
  } else if(auto lit_int = parse_literal_int(tokens, token)) {
    last_statement = Statement::NON_TERMINATED;
    node = std::move(*lit_int);
  } else if(auto lit_double = parse_literal_double(tokens, token)) {
    last_statement = Statement::NON_TERMINATED;
    node = std::move(*lit_double);
  } else if(auto lit_string = parse_literal_string(tokens, token)) {
    last_statement = Statement::NON_TERMINATED;
    node = std::move(*lit_string);
  } else if(auto op = parse_operator(tokens, token, nodes)) {
    last_statement = Statement::NON_TERMINATED;
    op->match([&node](ast::UnaryOperator& op) { node = std::move(op); },
              [&node](ast::BinaryOperator& op) { node = std::move(op); });
  } else if(auto scope = parse_scope(tokens, token)) {
    last_statement = Statement::TERMINATED;
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
  } else if(auto var = parse_variable(tokens, token)) {
    auto tmp = token;
    if(read_token(tokens, tmp, "=")) {
      last_statement = Statement::TERMINATED;
    } else {
      last_statement = Statement::NON_TERMINATED;
    }
    node = std::move(*var);
  } else {
    return {};
  }
  return node;
}

void parse_scope_internals(const Tokens& tokens, size_t& token,
                           ast::Scope& scope) {
  Statement last_statement = Statement::TERMINATED;

  while(token < tokens.size()) {
    if(read_token(tokens, token, ";")) {
      last_statement = Statement::TERMINATED;
    } else if(last_statement == Statement::NON_TERMINATED) {
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
        throw_unexprected_token(tokens, tmp);
      }
    }
  }
  if(last_statement == Statement::NON_TERMINATED) {
    UserSourceExc e;
    add_exception_info(tokens, token, e, [&] { e << "Expected a ';'"; });
    throw e;
  }
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

//////////////////////////////////////////
/// Variable parsing
//////////////////////////////////////////
core::optional<ast::Variable> parse_variable(const Tokens& tokens,
                                             size_t& token) {
  const static std::regex regex("([a-z][a-z0-9_]*)");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    expect_not_keyword(tokens, token);

    ast::Variable var(Token(tokens.at(token)));
    token = tmp;
    return var;
  }
  return {};
}

//////////////////////////////////////////
/// Return parsing
//////////////////////////////////////////
core::optional<ast::Return> parse_return(const Tokens& tokens, size_t& token) {
  auto tmp = token;
  try {
    if(read_token(tokens, tmp, "return")) {
      ast::Return ret(tokens.at(token));

      if(auto exe = parse_callable(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*exe));
      } else if(auto con = parse_condition(tokens, tmp)) {
        ret.output = std::make_unique<ast::ValueProducer>(std::move(*con));
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
      } else {
        throw_unexprected_token(tokens, tmp);
      }

      token = tmp;
      return ret;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "At return defined here:"; });
    std::throw_with_nested(e);
  }
  return {};
}

//////////////////////////////////////////
/// Break parsing
//////////////////////////////////////////
core::optional<ast::Break> parse_break(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  if(read_token(tokens, tmp, "break")) {
    ast::Break ret(tokens.at(token));

    token = tmp;
    return ret;
  }
  return {};
}

//////////////////////////////////////////
/// Operator parsing
//////////////////////////////////////////
template <typename T, typename is_Operator<T>::type>
void expect_operator_type(const char* const file, const size_t line,
                          const T& op) {
  using Operation =
      typename std::conditional<std::is_same<T, ast::UnaryOperator>::value,
                                ast::UnaryOperation,
                                ast::BinaryOperation>::type;
  if(op.operation == Operation::NONE) {
    OperatorExc e(file, line, "Missing operator");
    e << "There was no type specified for the operator:" << op;
    throw e;
  }
}

ast::ValueProducer node_to_value(ast::Scope::Node& node) {
  using namespace ast;
  using namespace callable;
  ast::ValueProducer value;

  node.match(
      [&value](Variable& e) { value = std::move(e); },
      [&value](Callable& e) { value = std::move(e); },
      [&value](UnaryOperator& e) { value = std::move(e); },
      [&value](BinaryOperator& e) { value = std::move(e); },
      [&value](Literal<Literals::BOOL>& e) { value = std::move(e); },
      [&value](Literal<Literals::INT>& e) { value = std::move(e); },
      [&value](Literal<Literals::DOUBLE>& e) { value = std::move(e); },
      [&value](Literal<Literals::STRING>& e) { value = std::move(e); },
      [](Break&) { throw_conversion(__FILE__, __LINE__, "Break"); },
      [](Define&) { throw_conversion(__FILE__, __LINE__, "Define"); },
      [](Return&) { throw_conversion(__FILE__, __LINE__, "Return"); },
      [](Scope&) { throw_conversion(__FILE__, __LINE__, "Scope"); },
      [](logic::If&) { throw_conversion(__FILE__, __LINE__, "If"); },
      [](loop::While&) { throw_conversion(__FILE__, __LINE__, "While"); },
      [](loop::DoWhile&) { throw_conversion(__FILE__, __LINE__, "DoWhile"); },
      [](loop::For&) { throw_conversion(__FILE__, __LINE__, "For"); });
  return value;
}

core::variant<ast::UnaryOperator, ast::BinaryOperator>
node_to_operator(const Tokens& tokens, ast::Scope::Node& node) {
  using namespace ast;

  ::core::variant<ast::UnaryOperator, ast::BinaryOperator> ret;
  ::core::optional<Token> token;

  node.match(
      [&ret](UnaryOperator& op) {
        expect_operator_type(__FILE__, __LINE__, op);
        ret = std::move(op);
      },
      [&ret](BinaryOperator& op) {
        expect_operator_type(__FILE__, __LINE__, op);
        ret = std::move(op);
      },
      [&token](Break& ele) { token = ele.token; },
      [&token](Variable& ele) { token = ele.token; },
      [&token](Define& ele) { token = ele.token; },
      [&token](callable::Callable& ele) { token = ele.token; },
      [&token](Return& ele) { token = ele.token; },
      [&token](Scope& ele) { token = ele.token; },
      [&token](logic::If& ele) { token = ele.token; },
      [&token](loop::While& ele) { token = ele.token; },
      [&token](loop::DoWhile& ele) { token = ele.token; },
      [&token](loop::For& ele) { token = ele.token; },
      [&token](Literal<Literals::BOOL>& ele) { token = ele.token; },
      [&token](Literal<Literals::INT>& ele) { token = ele.token; },
      [&token](Literal<Literals::DOUBLE>& ele) { token = ele.token; },
      [&token](Literal<Literals::STRING>& ele) { token = ele.token; });
  if(token) {
    throw_unexprected_token(*token, tokens.file);
  }
  return ret;
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

core::optional<ast::Variable> extract_var_def(ast::Scope::Node& node) {
  ::core::optional<ast::Variable> ret;

  node.match(
      [&ret](const ast::Define& def) {
        def.definition.match([&ret](const ast::Variable& var) { ret = var; },
                             [](const ast::callable::Function&) {},
                             [](const ast::callable::EntryFunction&) {});

      },                                                  //
      [](const ast::Break&) {},                           //
      [](const ast::Variable&) {},                        //
      [](const ast::callable::Callable&) {},              //
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

void setup_operator_wokespace(
    std::vector<ast::Scope::Node>& workspace,
    std::vector<ast::Scope::Node>& nodes,
    ::core::variant<ast::UnaryOperator, ast::BinaryOperator>& op) {
  op.match(
      [&workspace](ast::UnaryOperator& op) {
        expect_operator_type(__FILE__, __LINE__, op);
        workspace.push_back(std::move(op));
      },
      [&nodes, &workspace](ast::BinaryOperator& op) {
        expect_operator_type(__FILE__, __LINE__, op);
        if(nodes.size() > 0) {
          if(auto var = extract_var_def(nodes.back())) {
            workspace.push_back(std::move(*var));
          } else {
            workspace.push_back(std::move(nodes.back()));
            nodes.erase(nodes.end() - 1);
          }
        }
        workspace.push_back(std::move(op));
      });
}

void parse_operants(const Tokens& tokens, size_t& token,
                    std::vector<ast::Scope::Node>& workspace) {
  auto tmp = token;

  while(tmp < tokens.size()) {
    if(auto exe = parse_callable(tokens, tmp)) {
      workspace.push_back(std::move(*exe));
    } else if(auto lit_bool = parse_literal_bool(tokens, tmp)) {
      workspace.push_back(std::move(*lit_bool));
    } else if(auto lit_int = parse_literal_int(tokens, tmp)) {
      workspace.push_back(std::move(*lit_int));
    } else if(auto lit_double = parse_literal_double(tokens, tmp)) {
      workspace.push_back(std::move(*lit_double));
    } else if(auto lit_string = parse_literal_string(tokens, tmp)) {
      workspace.push_back(std::move(*lit_string));
    } else if(auto op = parse_operator_internals(tokens, tmp)) {
      op->match(
          [&workspace](ast::UnaryOperator& op) {
            workspace.push_back(std::move(op));
          },
          [&workspace](ast::BinaryOperator& op) {
            workspace.push_back(std::move(op));
          });
    } else if(read_token(tokens, tmp, "(")) {
      if(auto con = parse_condition(tokens, tmp)) {
        workspace.push_back(value_to_node(*con));
        expect_token(tokens, tmp, ")");
      } else {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e,
                           [&] { e << "Expected an expression."; });
        throw e;
      }
    } else if(auto var = parse_variable(tokens, tmp)) {
      workspace.push_back(std::move(*var));
    } else {
      break;  // We are done
    }
  }

  token = tmp;
}

void expect_operatees(const Token& token, const std::string& file,
                      const std::vector<ast::Scope::Node>::iterator& previous,
                      const std::vector<ast::Scope::Node>::iterator& next,
                      const std::vector<ast::Scope::Node>::iterator& end) {
  if(previous == end && next == end) {
    UserSourceExc e;
    add_exception_info(token, file, e, [&] {
      e << "Missing left and right hand token for binary operator '"
        << token.token << '\'';
    });
    throw e;
  }
  if(previous == end) {
    UserSourceExc e;
    add_exception_info(token, file, e, [&] {
      e << "Missing left hand token for binary operator '" << token.token
        << '\'';
    });
    throw e;
  }
  if(next == end) {
    UserSourceExc e;
    add_exception_info(token, file, e, [&] {
      e << "Missing right hand token for binary operator '" << token.token
        << '\'';
    });
    throw e;
  }
}

void assamble_unary(const std::string& file,
                    std::vector<ast::Scope::Node>& nodes,
                    std::vector<ast::Scope::Node>::iterator& next,
                    ast::UnaryOperator& op) {
  if(next == nodes.end()) {
    UserSourceExc e;
    add_exception_info(op.token, file, e, [&] {
      e << "Missing token for unary operator '" << op.token.token << '\'';
    });
    throw e;
  }
  op.operand = std::make_unique<ast::ValueProducer>(node_to_value(*next));
  nodes.erase(next);
}

void assamble_binary(const std::string& file,
                     std::vector<ast::Scope::Node>& nodes, size_t& index,
                     std::vector<ast::Scope::Node>::iterator& previous,
                     std::vector<ast::Scope::Node>::iterator& next,
                     ast::BinaryOperator& op) {
  expect_operatees(op.token, file, previous, next, nodes.end());

  op.left_operand =
      std::make_unique<ast::ValueProducer>(node_to_value(*previous));
  op.right_operand = std::make_unique<ast::ValueProducer>(node_to_value(*next));

  --index;  // we used the left
  nodes.erase(next);
  // We changed the vector - get new, VALID iterator
  previous = nodes.begin();
  std::advance(previous, index);  // we decremented index already
  nodes.erase(previous);
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
        assamble_unary(file, nodes, next, op);
      },
      [&file, &nodes, &index, &next, &previous](ast::BinaryOperator& op) {
        assamble_binary(file, nodes, index, previous, next, op);
      },
      [](ast::Break&) {},                            //
      [](ast::Variable&) {},                         //
      [](ast::Define&) {},                           //
      [](ast::callable::Callable&) {},               //
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
          expect_operator_type(__FILE__, __LINE__, op);

          if(op.operation == operaton && !op.operand) {
            assamble_operator(file, nodes, i);
          }
        },
        [](ast::Break&) {},                            //
        [](ast::Variable&) {},                         //
        [](ast::Define&) {},                           //
        [](ast::callable::Callable&) {},               //
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
          expect_operator_type(__FILE__, __LINE__, op);

          if(op.operation == operaton && !op.left_operand &&
             !op.right_operand) {
            assamble_operator(file, nodes, i);
          }
        },
        [](ast::Break&) {},                            //
        [](ast::Variable&) {},                         //
        [](ast::Define&) {},                           //
        [](ast::callable::Callable&) {},               //
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
  assamble_operators(file, nodes, ast::UnaryOperation::TYPEOF);

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
  assamble_operators(file, nodes, ast::UnaryOperation::PRINT);

  if(nodes.size() > 1) {
    throw_unexprected_token(node_to_token(nodes.at(1)), file);
  }
}

core::optional<ast::UnaryOperator> parse_unary_operator(const Tokens& tokens,
                                                        size_t& token) {
  auto op = ast::UnaryOperation::NONE;
  auto tmp = token;

  if(read_token(tokens, tmp, "!")) {
    op = ast::UnaryOperation::NOT;
  } else if(read_token(tokens, tmp, "typeof")) {
    op = ast::UnaryOperation::TYPEOF;
  } else if(read_token(tokens, tmp, "print")) {
    op = ast::UnaryOperation::PRINT;
  }

  if(op != ast::UnaryOperation::NONE) {
    ast::UnaryOperator un(tokens.at(token));
    un.operation = op;
    token = tmp;
    return un;
  }
  return {};
}
core::optional<ast::BinaryOperator> parse_binary_operator(const Tokens& tokens,
                                                          size_t& token) {
  auto op = ast::BinaryOperation::NONE;
  auto tmp = token;

  if(read_token(tokens, tmp, "/")) {
    op = ast::BinaryOperation::DIVIDE;
  } else if(read_token(tokens, tmp, "*")) {
    op = ast::BinaryOperation::MULTIPLY;
  } else if(read_token(tokens, tmp, "%")) {
    op = ast::BinaryOperation::MODULO;
  } else if(read_token(tokens, tmp, "+")) {
    op = ast::BinaryOperation::ADD;
  } else if(read_token(tokens, tmp, "-")) {
    op = ast::BinaryOperation::SUBTRACT;
  } else if(read_token(tokens, tmp, "<")) {
    op = ast::BinaryOperation::SMALLER;
  } else if(read_token(tokens, tmp, "<=")) {
    op = ast::BinaryOperation::SMALLER_EQUAL;
  } else if(read_token(tokens, tmp, ">")) {
    op = ast::BinaryOperation::GREATER;
  } else if(read_token(tokens, tmp, ">=")) {
    op = ast::BinaryOperation::GREATER_EQUAL;
  } else if(read_token(tokens, tmp, "==")) {
    op = ast::BinaryOperation::EQUAL;
  } else if(read_token(tokens, tmp, "!=")) {
    op = ast::BinaryOperation::NOT_EQUAL;
  } else if(read_token(tokens, tmp, "&&")) {
    op = ast::BinaryOperation::AND;
  } else if(read_token(tokens, tmp, "||")) {
    op = ast::BinaryOperation::OR;
  } else if(read_token(tokens, tmp, "=")) {
    op = ast::BinaryOperation::ASSIGNMENT;
  }

  if(op != ast::BinaryOperation::NONE) {
    ast::BinaryOperator bi(tokens.at(token));
    bi.operation = op;
    token = tmp;
    return bi;
  }
  return {};
}

core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>>
parse_operator_internals(const Tokens& tokens, size_t& token) {
  auto tmp = token;
  ::core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>> ret;

  if(auto un = parse_unary_operator(tokens, tmp)) {
    ret.emplace(std::move(*un));
    token = tmp;
  } else if(auto bi = parse_binary_operator(tokens, tmp)) {
    ret.emplace(std::move(*bi));
    token = tmp;
  }
  return ret;
}

core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>>
parse_operator(const Tokens& tokens, size_t& token,
               std::vector<ast::Scope::Node>& nodes) {
  auto tmp = token;
  ::core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>> ret;

  try {
    std::vector<ast::Scope::Node> workspace;

    if(auto op = parse_operator_internals(tokens, tmp)) {
      setup_operator_wokespace(workspace, nodes, *op);
      parse_operants(tokens, tmp, workspace);
      assamble_operator(tokens.file, workspace);

      if(workspace.size() == 1) {
        ret = node_to_operator(tokens, workspace.front());
      }
    }
  } catch(UserExc&) {
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

//////////////////////////////////////////
/// Condition parsing
//////////////////////////////////////////
ast::ValueProducer
assamble_conditions(const std::string& file,
                    std::vector<ast::Scope::Node> conditions) {
  assamble_operators(file, conditions, ast::UnaryOperation::NOT);
  assamble_operators(file, conditions, ast::UnaryOperation::TYPEOF);

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
    throw_unexprected_token(node_to_token(conditions.at(1)), file);
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
    } else if(auto var = parse_variable(tokens, tmp)) {
      conditions.push_back(std::move(*var));
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

//////////////////////////////////////////
/// If parsing
//////////////////////////////////////////
void parse_if_condition(const Tokens& tokens, size_t& token,
                        ast::logic::If& iff) {
  auto tmp = token;

  expect_token(tokens, tmp, "(");
  auto condition = parse_condition(tokens, tmp);
  if(!condition) {
    UserSourceExc e;
    add_exception_info(tokens, tmp, e, [&] { e << "Expected an expression."; });
    throw e;
  }

  expect_token(tokens, tmp, ")");

  token = tmp;
  iff.condition = std::make_unique<ast::ValueProducer>(std::move(*condition));
}

void parse_true(const Tokens& tokens, size_t& token, ast::logic::If& iff) {
  auto tmp = token;

  auto true_scope = parse_scope(tokens, tmp);

  if(!true_scope) {
    UserSourceExc e;
    add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
    throw e;
  }
  token = tmp;
  iff.true_scope = std::make_unique<ast::Scope>(std::move(*true_scope));
}

void parse_false(const Tokens& tokens, size_t& token, ast::logic::If& iff) {
  auto tmp = token;

  if(read_token(tokens, tmp, "else")) {
    try {
      auto scope = parse_scope(tokens, tmp);

      if(!scope) {
        UserSourceExc e;
        add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
        throw e;
      }

      token = tmp;
      iff.false_scope = std::make_unique<ast::Scope>(std::move(*scope));
    } catch(UserExc&) {
      UserTailExc e;
      add_exception_info(tokens, token, e,
                         [&e] { e << "In the else part defined here:"; });
      std::throw_with_nested(e);
    }
  }
}

core::optional<ast::logic::If> parse_if(const Tokens& tokens, size_t& token) {
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, "if")) {
      ast::logic::If iff(tokens.at(token));

      parse_if_condition(tokens, tmp, iff);
      parse_true(tokens, tmp, iff);

      if(tmp < tokens.size()) {
        parse_false(tokens, tmp, iff);
      }
      token = tmp;
      return iff;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "In the if defined here:"; });
    std::throw_with_nested(e);
  }
  return {};
}

//////////////////////////////////////////
/// While parsing
//////////////////////////////////////////
void parse_while_condition(const Tokens& tokens, size_t& token,
                           ast::loop::While& whi) {
  auto tmp = token;

  expect_token(tokens, tmp, "(");
  auto condition = parse_condition(tokens, tmp);
  if(!condition) {
    UserSourceExc e;
    add_exception_info(tokens, tmp, e, [&] { e << "Expected a condition."; });
    throw e;
  }
  whi.condition = std::make_unique<ast::ValueProducer>(std::move(*condition));
  expect_token(tokens, tmp, ")");
  token = tmp;
}

void parse_while_scope(const Tokens& tokens, size_t& token,
                       ast::loop::While& whi) {
  auto tmp = token;

  auto scope = parse_scope(tokens, tmp);
  if(!scope) {
    UserSourceExc e;
    add_exception_info(tokens, tmp, e, [&] { e << "Expected a scope."; });
    throw e;
  }
  whi.scope = std::make_unique<ast::Scope>(std::move(*scope));
  token = tmp;
}

core::optional<ast::loop::While> parse_while(const Tokens& tokens,
                                             size_t& token) {
  auto tmp = token;

  try {
    if(read_token(tokens, tmp, "while")) {
      ast::loop::While w(tokens.at(token));

      parse_while_condition(tokens, tmp, w);
      parse_while_scope(tokens, tmp, w);
      token = tmp;
      return w;
    }
  } catch(UserExc&) {
    UserTailExc e;
    add_exception_info(tokens, token, e,
                       [&e] { e << "In the while defined here:"; });
    std::throw_with_nested(e);
  }
  return {};
}
}

ast::Scope parse(std::string macro, std::string file_name) {
  Tokens tokens = {tokenizer::tokenize(macro), std::move(file_name)};
  auto root = ast::Scope(Token(0, 0, ""));

  for(size_t i = 0; i < tokens.size(); ++i) {
    parse_scope_internals(tokens, i, root);
  }

  // TODO check that no break is in a scope that is not contained by a
  // loop (functions reset the counter)
  // TODO check that no ast elements follow a break
  // TODO check that no ast elements follow a return
  // TODO check that a main function is given
  // TODO validate root scope - only definitions
  // TODO check unique parameter
  // TODO check unique pointer are set
  // TODO check left assignment operator is variable

  return root;
}
}
}
}
