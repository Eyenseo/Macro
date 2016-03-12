#include "cad/macro/parser/Tokenizer.h"

#include "cad/macro/parser/Token.h"

#include <regex>
#include <cassert>

#include <iostream>

namespace cad {
namespace macro {
namespace parser {
namespace tokenizer {
namespace {
struct Position {
  size_t line;
  size_t column;
  size_t string;
};

void token_begin(Macro macro, Position& position) {
  auto end = macro.size();

  while(position.string < end) {
    switch(macro[position.string]) {
    case ' ':
    case '\t':
    case '\v':
    case '\f':
      ++position.column;
      break;
    case '\n':
    case '\r':
      ++position.line;
      position.column = 1;
      break;
    default:
      end = 0;  // We are done!
    }

    if(end > 0) {
      ++position.string;
    }
  }
}

void normal_token_end(Macro macro, Position& position) {
  std::regex regex("([^a-zA-Z0-9])", std::regex::extended);
  std::smatch match;
  std::regex_search(macro.begin() + position.string, macro.end(), match, regex);

  if(match.empty()) {
    position.column += macro.size() - position.string;
    position.string = macro.size();
  } else {
    if(match.position(1) == 0) {
      position.column += 1;
      position.string += 1;
    } else {
      position.column += match.position(1);
      position.string += match.position(1);
    }
  }
}

void token_end(Macro macro, Position& position) {
  if(position.string + 2 <= macro.size()) {
    if((macro[position.string] == '&' && macro[position.string + 1] == '&') ||
       (macro[position.string] == '|' && macro[position.string + 1] == '|') ||
       (macro[position.string] == '=' && macro[position.string + 1] == '=') ||
       (macro[position.string] == '<' && macro[position.string + 1] == '=') ||
       (macro[position.string] == '>' && macro[position.string + 1] == '=')) {
      position.column += 2;
      position.string += 2;
    } else if(macro[position.string] == '>' || macro[position.string] == '<') {
      position.column += 1;
      position.string += 1;
    } else {
      normal_token_end(macro, position);
    }
  } else {
    normal_token_end(macro, position);
  }
}

Token next_string_token(Macro macro, Position& position) {
  const auto start = position.string;
  char last_token = ' ';
  auto end = macro.size();

  while(position.string < end) {
    ++position.string;

    switch(macro[position.string]) {
    case ' ':
    case '\t':
    case '\v':
    case '\f':
      ++position.column;
      break;
    case '\n':
    case '\r':
      ++position.line;
      position.column = 1;
      break;
    case '"':
      if(last_token != '\\') {
        ++position.string;
        end = 0;
      }
      ++position.column;
      break;
    default:
      ++position.column;
      continue;
    }
    last_token = macro[position.string];
  }

  const auto ret = Token(position.line, position.column,
                         macro.substr(start, position.string - start));
  return ret;
}

Token next_token(Macro macro, Position& position) {
  if(macro[position.string] == '\"') {
    return next_string_token(macro, position);
  } else {
    const auto start = position.string;

    token_end(macro, position);
    const auto ret = Token(position.line, position.column,
                           macro.substr(start, position.string - start));
    return ret;
  }
}
}

std::vector<Token> tokenize(Macro macro) {
  std::vector<Token> tokens;
  Position position = {0, 0, 0};
  token_begin(macro, position);

  while(position.string < macro.size()) {
    tokens.push_back(next_token(macro, position));
    token_begin(macro, position);
  }
  return tokens;
}
}
}
}
}
