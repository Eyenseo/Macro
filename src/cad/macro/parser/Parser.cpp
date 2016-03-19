#include "cad/macro/parser/Parser.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/Literal.h"
#include "cad/macro/ast/ValueProducer.h"
#include "cad/macro/parser/Tokenizer.h"

#include <core/optional.hpp>

#include <string>
#include <regex>
#include <cassert>

namespace cad {
namespace macro {
namespace parser {
namespace {
void parse_scope_internals(const std::vector<Token>& tokens, size_t& token,
                           ast::Scope& scope);

void expect_token(const std::vector<Token>& tokens, size_t& token,
                  const char* const token_literal) {
  if(tokens.at(token).token != token_literal) {
    // TODO throw
  }
  auto tmp = token + 1;
  if(tmp >= tokens.size()) {
    // TODO throw
  }
  token = tmp;
}

bool read_token(const std::vector<Token>& tokens, size_t& token,
                const char* const token_literal) {
  if(tokens.at(token).token != token_literal) {
    return false;
  }
  auto tmp = token + 1;
  if(tmp >= tokens.size()) {
    // TODO throw
  }
  token = tmp;
  return true;
}

bool read_token(const std::vector<Token>& tokens, size_t& token,
                const std::regex& token_regex) {
  std::smatch match;
  if(!std::regex_match(tokens.at(token).token, match, token_regex)) {
    return false;
  }
  auto tmp = token + 1;
  if(tmp >= tokens.size()) {
    // TODO throw
  }
  token = tmp;
  return true;
}

core::optional<ast::Literal<ast::Literals::BOOL>>
parse_literal_bool(const std::vector<Token>& tokens, size_t& token) {
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
parse_literal_int(const std::vector<Token>& tokens, size_t& token) {
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
parse_literal_double(const std::vector<Token>& tokens, size_t& token) {
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
core::optional<ast::Literal<ast::Literals::STRING>>
parse_literal_string(const std::vector<Token>& tokens, size_t& token) {
  const static std::regex regex("(\\\".*\\\")");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    ast::Literal<ast::Literals::STRING> lit(tokens.at(token));
    lit.data = lit.token.token.substr(1, lit.token.token.size() - 2);

    auto replace_all = [&lit](const std::string& search,
                              const std::string& replace) {
      for(size_t pos = 0;; pos += replace.length()) {
        pos = lit.data.find(search, pos);

        if(pos == std::string::npos) {
          break;
        }

        lit.data.erase(pos, search.length());
        lit.data.insert(pos, replace);
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

    token = tmp;
    return lit;
  }
  return {};
}

core::optional<ast::Scope> parse_scope(const std::vector<Token>& tokens,
                                       size_t& token) {
  auto tmp = token;
  if(read_token(tokens, tmp, "{")) {
    ast::Scope sub_scope(tokens.at(token));

    parse_scope_internals(tokens, tmp, sub_scope);
    // TODO catch - add information
    expect_token(tokens, tmp, "}");

    token = tmp;
    return sub_scope;
  }
  return {};
}

core::optional<ast::Variable> parse_variable(const std::vector<Token>& tokens,
                                             size_t& token) {
  const static std::regex regex("([a-z][a-z0-9_]*)");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    ast::Variable var(Token(tokens.at(token)));
    token = tmp;
    return var;
  }
  return {};
}

template <typename T,
          typename std::enable_if<
              std::is_same<T, ast::executable::Function>::value ||
                  std::is_same<T, ast::executable::EntryFunction>::value,
              bool>::type = true>
core::optional<T> parse_function_internals(const std::vector<Token>& tokens,
                                           size_t& token, T&& fun) {
  auto tmp = token;

  while(auto var = parse_variable(tokens, tmp)) {
    fun.parameter.push_back(std::move(*var));
    if(!read_token(tokens, tmp, ",")) {
      break;  // we do not expect another variable
    }
  }
  // TODO catch - add information
  expect_token(tokens, tmp, ")");

  auto fun_scope = parse_scope(tokens, tmp);
  if(!fun_scope) {
    // TODO throw
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

core::optional<ast::executable::EntryFunction>
parse_entry_function(const std::vector<Token>& tokens, size_t& token) {
  auto tmp = token;
  if(read_token(tokens, tmp, "main")) {
    // TODO catch - add information
    expect_token(tokens, tmp, "(");
    auto fun = parse_function_internals(
        tokens, tmp, ast::executable::EntryFunction(tokens.at(token)));
    token = tmp;
    return fun;
  }
  return {};
}

core::optional<ast::executable::Function>
parse_function(const std::vector<Token>& tokens, size_t& token) {
  const static std::regex regex("([a-z][a-z0-9_]*)");
  auto tmp = token;

  if(read_token(tokens, tmp, regex) && read_token(tokens, tmp, "(")) {
    auto fun = parse_function_internals(
        tokens, tmp, ast::executable::Function(tokens.at(token)));
    token = tmp;
    return fun;
  }
  return {};
}

core::optional<ast::Define> parse_definition(const std::vector<Token>& tokens,
                                             size_t& token) {
  auto tmp = token;
  if(read_token(tokens, tmp, "def")) {
    ast::Define def(tokens.at(token));

    if(auto entry_function = parse_entry_function(tokens, tmp)) {
      def.definition.emplace(std::move(*entry_function));
    } else if(auto function = parse_function(tokens, tmp)) {
      def.definition.emplace(std::move(*function));
    } else {
      // TODO throw
    }
    token = tmp;
    return def;
  } else if(read_token(tokens, tmp, "var")) {
    ast::Define def(tokens.at(token));

    if(auto variable = parse_variable(tokens, tmp)) {
      def.definition.emplace(std::move(*variable));
    } else {
      // TODO throw
    }
    token = tmp;
    return def;
  }
  return {};
}

core::optional<ast::executable::Executable>
parse_executable(const std::vector<Token>& tokens, size_t& token) {
  const static std::regex regex("([a-z][a-z0-9_]*)");

  auto tmp = token;
  if(read_token(tokens, tmp, regex) && read_token(tokens, tmp, "(")) {
    ast::executable::Executable exec(tokens.at(token));

    while(tmp < tokens.size()) {
      if(auto exe = parse_executable(tokens, tmp)) {
        exec.parameter.emplace_back(std::move(*exe));
      } else if(auto lit_bool = parse_literal_bool(tokens, tmp)) {
        exec.parameter.emplace_back(std::move(*lit_bool));
      } else if(auto lit_int = parse_literal_int(tokens, tmp)) {
        exec.parameter.emplace_back(std::move(*lit_int));
      } else if(auto lit_double = parse_literal_double(tokens, tmp)) {
        exec.parameter.emplace_back(std::move(*lit_double));
      } else if(auto lit_string = parse_literal_string(tokens, tmp)) {
        exec.parameter.emplace_back(std::move(*lit_string));
      } else if(auto var = parse_variable(tokens, tmp)) {
        exec.parameter.emplace_back(std::move(*var));
      } else {
        // TODO operator
        // TODO throw
      }
      if(!read_token(tokens, tmp, ",")) {
        break;  // we do not expect another variable
      }
    }
    // TODO catch - add information
    expect_token(tokens, tmp, ")");
    token = tmp;
    return exec;
  }
  return {};
}

core::optional<ast::Return> parse_return(const std::vector<Token>& tokens,
                                         size_t& token) {
  auto tmp = token;

  if(read_token(tokens, tmp, "return")) {
    ast::Return ret(tokens.at(token));

    if(auto exe = parse_executable(tokens, tmp)) {
      ret.output = std::make_unique<ast::ValueProducer>(std::move(*exe));
    } else if(auto lit_bool = parse_literal_bool(tokens, tmp)) {
      ret.output = std::make_unique<ast::ValueProducer>(std::move(*lit_bool));
    } else if(auto lit_int = parse_literal_int(tokens, tmp)) {
      ret.output = std::make_unique<ast::ValueProducer>(std::move(*lit_int));
    } else if(auto lit_double = parse_literal_double(tokens, tmp)) {
      ret.output = std::make_unique<ast::ValueProducer>(std::move(*lit_double));
    } else if(auto lit_string = parse_literal_string(tokens, tmp)) {
      ret.output = std::make_unique<ast::ValueProducer>(std::move(*lit_string));
    } else if(auto var = parse_variable(tokens, tmp)) {
      ret.output = std::make_unique<ast::ValueProducer>(std::move(*var));
    } else {
      // TODO operator
      // TODO throw
    }

    token = tmp;
    return ret;
  }
  return {};
}

core::optional<core::variant<ast::UnaryOperator, ast::BinaryOperator>>
parse_operator(const std::vector<Token>& tokens, size_t& token) {
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

void assamble_operator(std::vector<ast::ValueProducer>& conditions,
                       size_t& index) {
  auto next = conditions.begin();
  std::advance(next, index + 1);
  auto previous = conditions.begin();
  if(index == 0) {
    previous = conditions.end();
  } else {
    std::advance(previous, index - 1);
  }

  conditions.at(index).value.match(
      [&conditions, &next](ast::UnaryOperator& op) {
        if(next == conditions.end()) {
          // TODO throw
        }
        op.operand = std::make_unique<ast::ValueProducer>(std::move(*next));
        conditions.erase(next);
      },
      [&conditions, &index, &next, &previous](ast::BinaryOperator& op) {
        if(previous == conditions.end()) {
          // TODO throw
        }
        if(next == conditions.end()) {
          // TODO throw
        }
        op.left_operand =
            std::make_unique<ast::ValueProducer>(std::move(*previous));
        op.right_operand =
            std::make_unique<ast::ValueProducer>(std::move(*next));
        --index;
        conditions.erase(next);
        // We changed the vector - get new, VALID iterator
        previous = conditions.begin();
        std::advance(previous, index);  // we decremented index already
        conditions.erase(previous);
      },
      [](ast::Literal<ast::Literals::BOOL>&) {},
      [](ast::Literal<ast::Literals::INT>&) {},
      [](ast::Literal<ast::Literals::DOUBLE>&) {},
      [](ast::Literal<ast::Literals::STRING>&) {}, [](ast::Variable&) {},
      [](ast::executable::Executable&) {});
}

void assamble_operators(std::vector<ast::ValueProducer>& conditions,
                        const ast::UnaryOperation operaton) {
  for(size_t i = 0; i < conditions.size(); ++i) {
    conditions.at(i).value.match(
        [&conditions, &i, operaton](ast::UnaryOperator& op) {
          if(op.operation == ast::UnaryOperation::NONE) {
            // TODO throw
            assert(false && "ast::UnaryOperation::NONE");
          } else if(op.operation == operaton) {
            assamble_operator(conditions, i);
          }
        },
        [](ast::BinaryOperator&) {}, [](ast::Literal<ast::Literals::BOOL>&) {},
        [](ast::Literal<ast::Literals::INT>&) {},
        [](ast::Literal<ast::Literals::DOUBLE>&) {},
        [](ast::Literal<ast::Literals::STRING>&) {}, [](ast::Variable&) {},
        [](ast::executable::Executable&) {});
  }
}

void assamble_operators(std::vector<ast::ValueProducer>& conditions,
                        const ast::BinaryOperation operaton) {
  for(size_t i = 0; i < conditions.size(); ++i) {
    conditions.at(i).value.match(
        [&conditions, &i, operaton](ast::BinaryOperator& op) {
          if(op.operation == ast::BinaryOperation::NONE) {
            // TODO throw
            assert(false && "ast::BinaryOperation::NONE");
          } else if(op.operation == operaton) {
            assamble_operator(conditions, i);
          }
        },
        [](ast::UnaryOperator&) {}, [](ast::Literal<ast::Literals::BOOL>&) {},
        [](ast::Literal<ast::Literals::INT>&) {},
        [](ast::Literal<ast::Literals::DOUBLE>&) {},
        [](ast::Literal<ast::Literals::STRING>&) {}, [](ast::Variable&) {},
        [](ast::executable::Executable&) {});
  }
}

core::optional<ast::ValueProducer>
assamble_conditions(std::vector<ast::ValueProducer> conditions) {
  assamble_operators(conditions, ast::UnaryOperation::NOT);

  assamble_operators(conditions, ast::BinaryOperation::DIVIDE);
  assamble_operators(conditions, ast::BinaryOperation::MULTIPLY);
  assamble_operators(conditions, ast::BinaryOperation::MODULO);
  assamble_operators(conditions, ast::BinaryOperation::ADD);
  assamble_operators(conditions, ast::BinaryOperation::SUBTRACT);

  assamble_operators(conditions, ast::BinaryOperation::SMALLER);
  assamble_operators(conditions, ast::BinaryOperation::SMALLER_EQUAL);
  assamble_operators(conditions, ast::BinaryOperation::GREATER);
  assamble_operators(conditions, ast::BinaryOperation::GREATER_EQUAL);
  assamble_operators(conditions, ast::BinaryOperation::EQUAL);
  assamble_operators(conditions, ast::BinaryOperation::NOT_EQUAL);
  assamble_operators(conditions, ast::BinaryOperation::AND);
  assamble_operators(conditions, ast::BinaryOperation::OR);

  if(conditions.size() != 1) {
    for(const auto& c : conditions) {
      c.value.match(
          [](const ast::BinaryOperator& o) { std::cout << o; },
          [](const ast::UnaryOperator& o) { std::cout << o; },
          [](const ast::Literal<ast::Literals::BOOL>& o) { std::cout << o; },
          [](const ast::Literal<ast::Literals::INT>& o) { std::cout << o; },
          [](const ast::Literal<ast::Literals::DOUBLE>& o) { std::cout << o; },
          [](const ast::Literal<ast::Literals::STRING>& o) { std::cout << o; },
          [](const ast::Variable& o) { std::cout << o; },
          [](const ast::executable::Executable& o) { std::cout << o; });
    }

    assert(false && "Left over operators");
  }
  return conditions.front();
}

core::optional<ast::ValueProducer>
parse_condition(const std::vector<Token>& tokens, size_t& token) {
  auto tmp = token;

  std::vector<ast::ValueProducer> conditions;

  while(tmp < tokens.size()) {
    if(auto exe = parse_executable(tokens, tmp)) {
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
        // TODO throw
      }
      conditions.push_back(std::move(*condition));
      expect_token(tokens, tmp, ")");
    } else {
      break;  // We are done
    }
  }
  auto condition = assamble_conditions(std::move(conditions));
  if(!condition) {
    // TODO throw
  }
  token = tmp;
  return condition;
}

core::optional<ast::logic::If> parse_if(const std::vector<Token>& tokens,
                                        size_t& token) {
  auto tmp = token;

  if(read_token(tokens, tmp, "if")) {
    ast::logic::If iff(tokens.at(token));

    expect_token(tokens, tmp, "(");
    auto condition = parse_condition(tokens, tmp);
    if(!condition) {
      // TODO throw
    }
    iff.condition = std::make_unique<ast::ValueProducer>(std::move(*condition));
    expect_token(tokens, tmp, ")");

    auto true_scope = parse_scope(tokens, tmp);
    if(!true_scope) {
      // TODO throw
    }
    iff.true_scope = std::make_unique<ast::Scope>(std::move(*true_scope));

    if(tmp < tokens.size() && read_token(tokens, tmp, "else")) {
      auto false_scope = parse_scope(tokens, tmp);
      if(!false_scope) {
        // TODO throw
      }
      iff.false_scope = std::make_unique<ast::Scope>(std::move(*false_scope));
    }

    token = tmp;
    return iff;
  }
  return {};
}

void parse_scope_internals(const std::vector<Token>& tokens, size_t& token,
                           ast::Scope& scope) {
  if(auto def = parse_definition(tokens, token)) {
    scope.nodes.emplace_back(std::move(*def));
  } else if(auto iff = parse_if(tokens, token)) {
    scope.nodes.emplace_back(std::move(*iff));
  } else if(auto exe = parse_executable(tokens, token)) {
    scope.nodes.emplace_back(std::move(*exe));
  } else if(auto ret = parse_return(tokens, token)) {
    scope.nodes.emplace_back(std::move(*ret));
  } else {
    // TODO throw
  }
}
}

Parser::Parser() {
}

ast::Scope Parser::parse(std::string macro) const {
  const auto tokens = tokenizer::tokenize(macro);
  auto root = ast::Scope(Token(0, 0, ""));

  for(size_t i = 0; i < tokens.size(); ++i) {
    parse_scope_internals(tokens, i, root);
  }

  return root;
}
}
}
}
