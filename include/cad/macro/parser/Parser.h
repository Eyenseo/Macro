#ifndef cad_macro_parser_Parser_h
#define cad_macro_parser_Parser_h

#include <string>

// TODO remove
#include "cad/macro/parser/Tokenizer.h"
#include "cad/macro/ast/Scope.h"

#include <core/optional.hpp>

namespace cad {
namespace macro {
namespace ast {
class Scope;
}
}
}

namespace cad {
namespace macro {
namespace parser {
class Parser {
  static void expect_token(const std::vector<Token>& tokens, size_t& token,
                           const char* const token_literal) {
    if(tokens.at(token) != token_literal) {
      // TODO throw
    }
    auto tmp = token + 1;
    if(tmp >= tokens.size()) {
      // TODO throw
    }
    token = tmp;
  }
  static bool read_token(const std::vector<Token>& tokens, size_t& token,
                         const char* const token_literal) {
    if(tokens.at(token) != token_literal) {
      return false;
    }
    auto tmp = token + 1;
    if(tmp >= tokens.size()) {
      // TODO throw
    }
    token = tmp;
    return true;
  }

  static void parse_scope_internals(const std::vector<Token>& tokens,
                                    size_t& token, ast::Scope& scope) {
    if(parse_definition(tokens, token, scope)) {
    }
  }

  static core::optional<ast::Scope>
  parse_scope(const std::vector<Token>& tokens, size_t& token) {
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

  static core::optional<ast::executable::EntryFunction>
  parse_entry_function(const std::vector<Token>& tokens, size_t& token) {
    auto tmp = token;
    if(read_token(tokens, tmp, "main")) {
      ast::executable::EntryFunction fun(tokens.at(token));

      expect_token(tokens, tmp, "(");
      // TODO read parameters
      expect_token(tokens, tmp, ")");

      auto fun_scope = parse_scope(tokens, tmp);
      if(!fun_scope) {
        // TODO throw
      }
      fun.set_scope(*std::move(fun_scope));
      token = tmp;
      return fun;
    }
    return {};
  }

  static bool parse_definition(const std::vector<Token>& tokens, size_t& token,
                               ast::Scope& scope) {
    auto tmp = token;
    if(read_token(tokens, tmp, "def")) {
      ast::Define def(tokens.at(token));
      if(auto entry_function = parse_entry_function(tokens, tmp)) {
        def.define(std::move(*entry_function));
      }
      // TODO other
      scope.append(std::move(def));
      token = tmp;
      return true;
    }
    return false;
  }

public:
  Parser() {
  }

  ast::Scope parse(std::string macro) const {
    auto tokens = tokenizer::tokenize(macro);
    auto root = ast::Scope(Token(0, 0, ""));

    for(size_t i = 0; i < tokens.size(); ++i) {
      parse_scope_internals(tokens, i, root);
    }

    return root;
  }
};
}
}
}
#endif
