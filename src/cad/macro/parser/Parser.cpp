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
core::optional<ast::ValueProducer>
parse_condition(const std::vector<Token>& tokens, size_t& token);

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

core::optional<ast::Define>
parse_executable_definition(const std::vector<Token>& tokens, size_t& token) {
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
  }
  return {};
}
core::optional<ast::Define>
parse_variable_definition(const std::vector<Token>& tokens, size_t& token) {
  auto tmp = token;
  if(read_token(tokens, tmp, "var")) {
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
      } else if(auto con = parse_condition(tokens, tmp)) {
        exec.parameter.emplace_back(std::move(*con));
      } else {
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
    } else if(auto con = parse_condition(tokens, tmp)) {
      ret.output = std::make_unique<ast::ValueProducer>(std::move(*con));
    } else {
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

ast::ValueProducer node_to_value(ast::Scope::Node& node) {
  ast::ValueProducer value;
  node.match(
      [&value](ast::Variable& e) { value = std::move(e); },
      [&value](ast::Define&) {
        // TODO throw
        assert(false && "Not a ValueProducer");
      },
      [&value](ast::executable::EntryFunction&) {
        // TODO throw
        assert(false && "Not a ValueProducer");
      },
      [&value](ast::executable::Executable& e) { value = std::move(e); },
      [&value](ast::executable::Function&) {
        // TODO throw
        assert(false && "Not a ValueProducer");
      },
      [&value](ast::Return&) {
        // TODO throw
        assert(false && "Not a ValueProducer");
      },
      [&value](ast::Scope&) {
        // TODO throw
        assert(false && "Not a ValueProducer");
      },
      [&value](ast::UnaryOperator& e) { value = std::move(e); },
      [&value](ast::BinaryOperator& e) { value = std::move(e); },
      [&value](ast::logic::If&) {
        // TODO throw
        assert(false && "Not a ValueProducer");
      },
      [&value](ast::loop::While&) {
        // TODO throw
        assert(false && "Not a ValueProducer");
      },
      [&value](ast::loop::DoWhile&) {
        // TODO throw
        assert(false && "Not a ValueProducer");
      },
      [&value](ast::loop::For&) {
        // TODO throw
        assert(false && "Not a ValueProducer");
      },
      [&value](ast::Literal<ast::Literals::BOOL>& e) { value = std::move(e); },
      [&value](ast::Literal<ast::Literals::INT>& e) { value = std::move(e); },
      [&value](ast::Literal<ast::Literals::DOUBLE>& e) {
        value = std::move(e);
      },
      [&value](ast::Literal<ast::Literals::STRING>& e) {
        value = std::move(e);
      });
  return value;
}

ast::Scope::Node value_to_node(ast::ValueProducer& producer) {
  ast::Scope::Node node;
  producer.value.match(
      [&node](ast::Variable& e) { node = std::move(e); },
      [&node](ast::executable::Executable& e) { node = std::move(e); },
      [&node](ast::UnaryOperator& e) { node = std::move(e); },
      [&node](ast::BinaryOperator& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::BOOL>& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::INT>& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::DOUBLE>& e) { node = std::move(e); },
      [&node](ast::Literal<ast::Literals::STRING>& e) { node = std::move(e); });
  return node;
}

void assamble_operator(std::vector<ast::Scope::Node>& nodes, const size_t start,
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
      [&nodes, &next](ast::UnaryOperator& op) {
        if(next == nodes.end()) {
          // TODO throw
        }
        op.operand = std::make_unique<ast::ValueProducer>(node_to_value(*next));
        nodes.erase(next);
      },
      [&nodes, &index, &next, &previous](ast::BinaryOperator& op) {
        if(previous == nodes.end()) {
          // TODO throw
        }
        if(next == nodes.end()) {
          // TODO throw
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
      [](ast::Variable&) {},                         //
      [](ast::Define&) {},                           //
      [](ast::executable::EntryFunction&) {},        //
      [](ast::executable::Executable&) {},           //
      [](ast::executable::Function&) {},             //
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

void assamble_operators(std::vector<ast::Scope::Node>& nodes,
                        const size_t start,
                        const ast::UnaryOperation operaton) {
  for(size_t i = start; i < nodes.size(); ++i) {
    nodes.at(i).match(
        [&nodes, start, &i, operaton](ast::UnaryOperator& op) {
          if(op.operation == ast::UnaryOperation::NONE) {
            // TODO throw
            assert(false && "ast::UnaryOperation::NONE");
          } else if(op.operation == operaton) {
            assamble_operator(nodes, start, i);
          }
        },
        [](ast::Variable&) {},                         //
        [](ast::Define&) {},                           //
        [](ast::executable::EntryFunction&) {},        //
        [](ast::executable::Executable&) {},           //
        [](ast::executable::Function&) {},             //
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

void assamble_operators(std::vector<ast::Scope::Node>& nodes,
                        const size_t start,
                        const ast::BinaryOperation operaton) {
  for(size_t i = start; i < nodes.size(); ++i) {
    nodes.at(i).match(
        [&nodes, start, &i, operaton](ast::BinaryOperator& op) {
          if(op.operation == ast::BinaryOperation::NONE) {
            // TODO throw
            assert(false && "ast::BinaryOperation::NONE");
          } else if(op.operation == operaton) {
            assamble_operator(nodes, start, i);
          }
        },
        [](ast::Variable&) {},                         //
        [](ast::Define&) {},                           //
        [](ast::executable::EntryFunction&) {},        //
        [](ast::executable::Executable&) {},           //
        [](ast::executable::Function&) {},             //
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
assamble_conditions(std::vector<ast::Scope::Node> conditions) {
  assamble_operators(conditions, 0, ast::UnaryOperation::NOT);

  assamble_operators(conditions, 0, ast::BinaryOperation::DIVIDE);
  assamble_operators(conditions, 0, ast::BinaryOperation::MULTIPLY);
  assamble_operators(conditions, 0, ast::BinaryOperation::MODULO);
  assamble_operators(conditions, 0, ast::BinaryOperation::ADD);
  assamble_operators(conditions, 0, ast::BinaryOperation::SUBTRACT);

  assamble_operators(conditions, 0, ast::BinaryOperation::SMALLER);
  assamble_operators(conditions, 0, ast::BinaryOperation::SMALLER_EQUAL);
  assamble_operators(conditions, 0, ast::BinaryOperation::GREATER);
  assamble_operators(conditions, 0, ast::BinaryOperation::GREATER_EQUAL);
  assamble_operators(conditions, 0, ast::BinaryOperation::EQUAL);
  assamble_operators(conditions, 0, ast::BinaryOperation::NOT_EQUAL);
  assamble_operators(conditions, 0, ast::BinaryOperation::AND);
  assamble_operators(conditions, 0, ast::BinaryOperation::OR);

  if(conditions.size() != 1) {
    for(const auto& c : conditions) {
      c.match(
          [](const ast::Variable& e) { std::cout << e; },
          [](const ast::Define& e) { std::cout << e; },
          [](const ast::executable::EntryFunction& e) { std::cout << e; },
          [](const ast::executable::Executable& e) { std::cout << e; },
          [](const ast::executable::Function& e) { std::cout << e; },
          [](const ast::Return& e) { std::cout << e; },
          [](const ast::Scope& e) { std::cout << e; },
          [](const ast::UnaryOperator& e) { std::cout << e; },
          [](const ast::BinaryOperator& e) { std::cout << e; },
          [](const ast::logic::If& e) { std::cout << e; },
          [](const ast::loop::While& e) { std::cout << e; },
          [](const ast::loop::DoWhile& e) { std::cout << e; },
          [](const ast::loop::For& e) { std::cout << e; },
          [](const ast::Literal<ast::Literals::BOOL>& e) { std::cout << e; },
          [](const ast::Literal<ast::Literals::INT>& e) { std::cout << e; },
          [](const ast::Literal<ast::Literals::DOUBLE>& e) { std::cout << e; },
          [](const ast::Literal<ast::Literals::STRING>& e) { std::cout << e; });
    }

    assert(false && "Left over operators");
  }
  return node_to_value(conditions.front());
}

core::optional<ast::ValueProducer>
parse_condition(const std::vector<Token>& tokens, size_t& token) {
  std::vector<ast::Scope::Node> conditions;
  auto tmp = token;

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
      conditions.push_back(value_to_node(*condition));
      expect_token(tokens, tmp, ")");
    } else {
      break;  // We are done
    }
  }
  if(!conditions.empty()) {
    auto condition = assamble_conditions(std::move(conditions));
    token = tmp;
    return condition;
  } else {
    return {};
  }
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

core::optional<ast::loop::While> parse_while(const std::vector<Token>& tokens,
                                             size_t& token) {
  auto tmp = token;

  if(read_token(tokens, tmp, "while")) {
    ast::loop::While w(tokens.at(token));

    expect_token(tokens, tmp, "(");
    auto condition = parse_condition(tokens, tmp);
    if(!condition) {
      // TODO throw
    }
    w.condition = std::make_unique<ast::ValueProducer>(std::move(*condition));
    expect_token(tokens, tmp, ")");

    auto scope = parse_scope(tokens, tmp);
    if(!scope) {
      // TODO throw
    }
    w.scope = std::make_unique<ast::Scope>(std::move(*scope));

    token = tmp;
    return w;
  }
  return {};
}

void two_step_define_assign(std::vector<ast::Scope::Node>& nodes,
                            std::vector<ast::Scope::Node>::iterator current,
                            size_t& index, ast::Define& e) {
  if(!e.definition) {
    // TODO throw
    assert(false && "Empty definition");
  }
  e.definition->match([](ast::executable::EntryFunction&) {},  //
                      [&nodes, &index, &current](ast::Variable& v) {
                        nodes.emplace(current, v);
                        ++index;
                      },                                 //
                      [](ast::executable::Function&) {}  //
                      );
}

void two_step_define_assign(std::vector<ast::Scope::Node>& nodes,
                            std::vector<ast::Scope::Node>::iterator previous,
                            std::vector<ast::Scope::Node>::iterator current,
                            size_t& index) {
  if(previous == nodes.end()) {
    // TODO throw
  }
  previous->match(
      [&nodes, &index, &current](ast::Define& e) {
        two_step_define_assign(nodes, current, index, e);
      },                                             //
      [](ast::Variable&) {},                         //
      [](ast::executable::EntryFunction&) {},        //
      [](ast::executable::Executable&) {},           //
      [](ast::executable::Function&) {},             //
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

void two_step_define_assign(std::vector<ast::Scope::Node>& nodes,
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
      [&nodes, &index, &current, &previous](ast::BinaryOperator&) {
        two_step_define_assign(nodes, previous, current, index);
      },
      [](ast::Variable&) {},                         //
      [](ast::Define&) {},                           //
      [](ast::UnaryOperator&) {},                    //
      [](ast::executable::EntryFunction&) {},        //
      [](ast::executable::Executable&) {},           //
      [](ast::executable::Function&) {},             //
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

void two_step_define_assign(std::vector<ast::Scope::Node>& nodes,
                            const size_t start) {
  for(size_t i = start; i < nodes.size(); ++i) {
    nodes.at(i).match(
        [&nodes, start, &i](ast::BinaryOperator& op) {
          if(op.operation == ast::BinaryOperation::NONE) {
            // TODO throw
            assert(false && "ast::BinaryOperation::NONE");
          } else if(op.operation == ast::BinaryOperation::ASSIGNMENT) {
            two_step_define_assign(nodes, start, i);
          }
        },
        [](ast::Variable&) {},                         //
        [](ast::Define&) {},                           //
        [](ast::executable::EntryFunction&) {},        //
        [](ast::executable::Executable&) {},           //
        [](ast::executable::Function&) {},             //
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

void assamble_statement(ast::Scope& scope, const size_t last) {
  two_step_define_assign(scope.nodes, last);

  assamble_operators(scope.nodes, last, ast::UnaryOperation::NOT);

  assamble_operators(scope.nodes, last, ast::BinaryOperation::DIVIDE);
  assamble_operators(scope.nodes, last, ast::BinaryOperation::MULTIPLY);
  assamble_operators(scope.nodes, last, ast::BinaryOperation::MODULO);
  assamble_operators(scope.nodes, last, ast::BinaryOperation::ADD);
  assamble_operators(scope.nodes, last, ast::BinaryOperation::SUBTRACT);

  assamble_operators(scope.nodes, last, ast::BinaryOperation::SMALLER);
  assamble_operators(scope.nodes, last, ast::BinaryOperation::SMALLER_EQUAL);
  assamble_operators(scope.nodes, last, ast::BinaryOperation::GREATER);
  assamble_operators(scope.nodes, last, ast::BinaryOperation::GREATER_EQUAL);
  assamble_operators(scope.nodes, last, ast::BinaryOperation::EQUAL);
  assamble_operators(scope.nodes, last, ast::BinaryOperation::NOT_EQUAL);
  assamble_operators(scope.nodes, last, ast::BinaryOperation::AND);
  assamble_operators(scope.nodes, last, ast::BinaryOperation::OR);

  assamble_operators(scope.nodes, last, ast::BinaryOperation::ASSIGNMENT);
}

core::optional<ast::Scope::Node>
parse_scope_internals(const std::vector<Token>& tokens, size_t& token,
                      bool& new_statement) {
  ast::Scope::Node node;

  if(auto def = parse_executable_definition(tokens, token)) {
    if(!new_statement) {
      // TODO throw
      assert(false && "Not new statement");
    }
    node = std::move(*def);
  } else if(auto def = parse_variable_definition(tokens, token)) {
    if(!new_statement) {
      // TODO throw
      assert(false && "Not new statement");
    }
    new_statement = false;
    node = std::move(*def);
  } else if(auto iff = parse_if(tokens, token)) {
    if(!new_statement) {
      // TODO throw
      assert(false && "Not new statement");
    }
    node = std::move(*iff);
  } else if(auto whi = parse_while(tokens, token)) {
    if(!new_statement) {
      // TODO throw
      assert(false && "Not new statement");
    }
    node = std::move(*whi);
  } else if(auto exe = parse_executable(tokens, token)) {
    if(!new_statement) {
      // TODO throw
      assert(false && "Not new statement");
    }
    new_statement = false;
    node = std::move(*exe);
  } else if(auto ret = parse_return(tokens, token)) {
    if(!new_statement) {
      // TODO throw
      assert(false && "Not new statement");
    }
    new_statement = false;
    node = std::move(*ret);
  } else if(auto lit_bool = parse_literal_bool(tokens, token)) {
    node = std::move(*lit_bool);
    new_statement = false;
  } else if(auto lit_int = parse_literal_int(tokens, token)) {
    node = std::move(*lit_int);
  } else if(auto lit_double = parse_literal_double(tokens, token)) {
    new_statement = false;
    node = std::move(*lit_double);
  } else if(auto lit_string = parse_literal_string(tokens, token)) {
    node = std::move(*lit_string);
    new_statement = false;
  } else if(auto op = parse_operator(tokens, token)) {
    op->match([&node](ast::UnaryOperator& op) { node = std::move(op); },
              [&node](ast::BinaryOperator& op) { node = std::move(op); });
    new_statement = false;
  } else if(auto var = parse_variable(tokens, token)) {
    node = std::move(*var);
    new_statement = false;
  } else {
    return {};
  }
  return node;
}

void parse_scope_internals(const std::vector<Token>& tokens, size_t& token,
                           ast::Scope& scope) {
  auto last = scope.nodes.size();
  bool new_statement = true;

  while(token < tokens.size()) {
    if(read_token(tokens, token, ";")) {
      assamble_statement(scope, last);
      last = scope.nodes.size();
      new_statement = true;
    } else if(auto node = parse_scope_internals(tokens, token, new_statement)) {
      scope.nodes.push_back(std::move(*node));
    } else {
      auto tmp = token;  // No advance - that is done by the scope
      if(read_token(tokens, tmp, "}")) {
        break;  // done with the scope
      } else {
        // TODO throw
        std::cout << tokens.at(token) << "\n";
        assert(false && "Bad Token");
      }
    }
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
