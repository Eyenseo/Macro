#include "cad/macro/parser/Parser.h"

#include "cad/macro/ast/Scope.h"
#include "cad/macro/parser/Tokenizer.h"

#include <core/optional.hpp>

#include <string>
#include <regex>

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
  if(!std::regex_search(tokens.at(token).token, match, token_regex)) {
    return false;
  }
  auto tmp = token + 1;
  if(tmp >= tokens.size()) {
    // TODO throw
  }
  token = tmp;
  return true;
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
  for(const auto& v : fun.parameter) {
    v.match(
        [&fun_scope](ast::Variable p) {
          ast::Define def(p.token);
          def.definition.emplace(std::move(p));
          fun_scope->nodes.emplace_back(std::move(def));
        },
        [](const ast::executable::Executable&) {
          // TODO throw
        });
  }

  fun.scope = std::make_unique<ast::Scope>(*std::move(fun_scope));
  token = tmp;
  return fun;
}

core::optional<ast::executable::EntryFunction>
parse_entry_function_definition(const std::vector<Token>& tokens,
                                size_t& token) {
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
parse_function_definition(const std::vector<Token>& tokens, size_t& token) {
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

core::optional<ast::Variable>
parse_variable_definition(const std::vector<Token>& tokens, size_t& token) {
  const static std::regex regex("([a-z][a-z0-9_]*)");
  auto tmp = token;

  if(read_token(tokens, tmp, regex)) {
    ast::Variable var(tokens.at(token));
    if(read_token(tokens, tmp, "(")) {
      // TODO throw
    }
    token = tmp;
    return var;
  }
  return {};
}

core::optional<ast::Define> parse_definition(const std::vector<Token>& tokens,
                                             size_t& token) {
  auto tmp = token;
  if(read_token(tokens, tmp, "def")) {
    ast::Define def(tokens.at(token));

    if(auto entry_function = parse_entry_function_definition(tokens, tmp)) {
      def.definition.emplace(std::move(*entry_function));
    } else if(auto function = parse_function_definition(tokens, tmp)) {
      def.definition.emplace(std::move(*function));
    } else {
      // TODO throw
    }
    token = tmp;
    return def;
  } else if(read_token(tokens, tmp, "var")) {
    ast::Define def(tokens.at(token));

    if(auto variable = parse_variable_definition(tokens, tmp)) {
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

    while(token <= tokens.size()) {
      if(auto var = parse_executable(tokens, tmp)) {
        exec.parameter.push_back(std::move(*var));
      } else if(auto var = parse_variable(tokens, tmp)) {
        exec.parameter.push_back(std::move(*var));
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

    if(auto var = parse_executable(tokens, tmp)) {
      ret.output.emplace(std::move(*var));
    } else if(auto var = parse_variable(tokens, tmp)) {
      ret.output.emplace(std::move(*var));
    } else {
      // TODO throw
    }

    token = tmp;
    return ret;
  }
  return {};
}

void parse_scope_internals(const std::vector<Token>& tokens, size_t& token,
                           ast::Scope& scope) {
  if(auto def = parse_definition(tokens, token)) {
    scope.nodes.emplace_back(std::move(*def));
  } else if(auto exec = parse_executable(tokens, token)) {
    scope.nodes.emplace_back(std::move(*exec));
  } else if(auto ret = parse_return(tokens, token)) {
    scope.nodes.emplace_back(std::move(*ret));
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
