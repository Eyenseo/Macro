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
  size_t line_start;
  std::shared_ptr<std::string> source_line;
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
      position.source_line->append(macro.substr(
          position.line_start, position.string - position.line_start));
      position.line_start = position.string + 1;
      position.source_line = std::make_shared<std::string>();
      break;
    default:
      end = 0;  // We are done!
    }

    if(end > 0) {  // no need to advance - we are at an unprocessed character
      ++position.string;
    }
  }
}

bool float_token_end(Macro macro, Position& position) {
  // TODO see http://en.cppreference.com/w/cpp/string/basic_string/stof
  const static std::regex regex("(\\d*\\.\\d+)");
  std::smatch match;
  std::regex_search(macro.begin() + position.string, macro.end(), match, regex);

  if(!match.empty() && match.position(1) == 0) {
    position.column += match[1].length();
    position.string += match[1].length();
    return true;
  }
  return false;
}
void normal_token_end(Macro macro, Position& position) {
  // TODO see http://en.cppreference.com/w/cpp/string/basic_string/stol
  const static std::regex regex("([^a-zA-Z0-9_])");
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
  const auto current = macro[position.string];
  const auto next =
      (position.string + 1 <= macro.size()) ? macro[position.string + 1] : '\0';

  if((current == '&' && next == '&') || (current == '|' && next == '|') ||
     (current == '=' && next == '=') || (current == '!' && next == '=') ||
     (current == '<' && next == '=') || (current == '>' && next == '=')) {
    position.column += 2;
    position.string += 2;
  } else if(float_token_end(macro, position)) {
  } else {
    normal_token_end(macro, position);
  }
}

Token next_string_token(Macro macro, Position& position) {
  const auto start = position.string;
  char last_token = '\0';
  auto end = macro.size();

  while(position.string < end) {
    ++position.string;

    switch(macro[position.string]) {
    case ' ':
    case '\t':
    case '\v':
    case '\f':
      ++position.column;
      last_token = macro[position.string];
      break;
    case '\n':
    case '\r':
      ++position.line;
      position.column = 1;
      last_token = macro[position.string];

      position.source_line->append(macro.substr(
          position.line_start, position.string - position.line_start));
      position.line_start = position.string + 1;
      position.source_line = std::make_shared<std::string>();
      break;
    case '"':
      if(last_token != '\\') {
        ++position.string;
        end = 0;
      }
      ++position.column;
      last_token = macro[position.string];
      break;
    case '\\':
      if(last_token != '\\') {
        ++position.string;
      }
      last_token = '\0';  // We consumed the last escape
      ++position.column;
      break;
    default:
      ++position.column;
      last_token = macro[position.string];
      continue;
    }
  }

  const auto ret =
      Token(position.line, position.column,
            macro.substr(start, position.string - start), position.source_line);
  return ret;
}

Token next_token(Macro macro, Position& position) {
  if(macro[position.string] == '\"') {
    return next_string_token(macro, position);
  } else {
    Position tmp = position;

    token_end(macro, tmp);
    const auto ret =
        Token(position.line, position.column,
              macro.substr(position.string, tmp.string - position.string),
              position.source_line);
    position = tmp;
    return ret;
  }
}
}

std::vector<Token> tokenize(Macro macro) {
  std::vector<Token> tokens;
  Position position = {1, 1, 0, 0, std::make_shared<std::string>()};
  token_begin(macro, position);

  // TODO add line string to Token

  while(position.string < macro.size()) {
    tokens.push_back(next_token(macro, position));
    token_begin(macro, position);
  }
  position.source_line->append(
      macro.substr(position.line_start, macro.size() - position.line_start));

  return tokens;
}
}
}
}
}
