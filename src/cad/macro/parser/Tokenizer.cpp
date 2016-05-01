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
/**
 * @brief  Helper struct that tracks the position information in the string
 */
struct Position {
  size_t line;
  size_t column;
  size_t string;
  size_t line_start;
  std::shared_ptr<std::string> source_line;
};

/**
 * @brief  The functions finds the begin of a token - it eats all whitespace
 *
 * @param  macro     The macro
 * @param  position  The position
 */
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

/**
 * @brief  The function finds the end of a floating point number
 *
 * @param  macro     The macro
 * @param  position  The position
 *
 * @return true if the token was indeed a floating point number
 */
bool float_token_end(Macro macro, Position& position) {
  // TODO see http://en.cppreference.com/w/cpp/string/basic_string/stof
  const static std::regex regex("^(\\d*\\.\\d+)");
  std::smatch match;
  std::regex_search(macro.begin() + position.string, macro.end(), match, regex);

  if(!match.empty() && match.position(1) == 0) {
    position.column += match[1].length();
    position.string += match[1].length();
    return true;
  }
  return false;
}
/**
 * @brief  The function find the end of a normal Token, that can be a integer
 *         number
 *
 * @param  macro     The macro
 * @param  position  The position
 */
void normal_token_end(Macro macro, Position& position) {
  // TODO see http://en.cppreference.com/w/cpp/string/basic_string/stol
  const static std::regex regex("([^a-zA-Z0-9_])");
  std::smatch match;
  std::regex_search(macro.begin() + position.string, macro.end(), match, regex);

  if(match.empty()) {
    // We're done - with you, everything and life...
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

/**
 * @brief  The function finds the end of the Token
 *
 * @param  macro     The macro
 * @param  position  The position
 */
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
/**
 * @brief  The function tokenizes a string
 *
 * @param  macro     The macro
 * @param  position  The position
 *
 * @return String Token
 */
Token next_string_token(Macro macro, Position& position) {
  auto start = position;
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
        ++position.string;  // Move to the next character
        ++position.column;  // Move to the next character
        end = 0;
      }
      ++position.column;
      last_token = macro[position.string];
      break;
    case '\\':
      if(last_token != '\\') {
        last_token = '\\';
      } else {
        last_token = '\0';  // We consumed the last escape
      }
      ++position.column;
      break;
    default:
      ++position.column;
      last_token = macro[position.string];
      continue;
    }
  }

  const auto ret =
      Token(start.line, start.column,
            macro.substr(start.string, position.string - start.string),
            start.source_line);
  return ret;
}

/**
 * @brief  The function reads over an in-line comment
 *
 * @param  macro     The macro
 * @param  position  The position
 *
 * @return true if it read a in-line comment
 */
bool ignore_in_line_comment(Macro macro, Position& position) {
  auto end = macro.size();
  auto current = macro[position.string];
  auto next = (position.string + 1 <= end) ? macro[position.string + 1] : '\0';

  if(current == '/' && next == '*') {
    position.string += 2;
    position.column += 2;

    while(position.string < end) {
      current = macro[position.string];
      next = (position.string + 1 <= end) ? macro[position.string + 1] : '\0';

      if(current == '\n' || current == '\r') {
        position.source_line->append(macro.substr(
            position.line_start, position.string - position.line_start));
        position.line_start = position.string + 1;
        position.source_line = std::make_shared<std::string>();

        ++position.line;
        position.column = 0;  // will be advanced a few lines down
      } else if(current == '*' && next == '/') {
        position.string += 2;
        position.column += 2;
        break;
      }
      ++position.column;
      ++position.string;
    }
    return true;
  }
  return false;
}

/**
 * @brief  The function reads a line comment
 *
 * @param  macro     The macro
 * @param  position  The position
 *
 * @return true if it read a line comment
 */
bool ignore_line_comment(Macro macro, Position& position) {
  auto end = macro.size();
  const auto current = macro[position.string];
  const auto next =
      (position.string + 1 <= end) ? macro[position.string + 1] : '\0';

  if(current == '/' && next == '/') {
    position.string += 2;
    while(position.string < end) {
      if(macro[position.string] == '\n' || macro[position.string] == '\r') {
        position.source_line->append(macro.substr(
            position.line_start, position.string - position.line_start));
        position.line_start = position.string + 1;
        position.source_line = std::make_shared<std::string>();

        ++position.string;
        ++position.line;
        position.column = 1;
        break;
      }
      ++position.string;
    }
    return true;
  }
  return false;
}

/**
 * @brief  Tokenizes the next Token
 *
 * @param  macro     The macro
 * @param  position  The position
 *
 * @return Tokenized token
 */
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

  while(position.string < macro.size()) {
    if(!ignore_line_comment(macro, position) &&
       !ignore_in_line_comment(macro, position)) {
      tokens.push_back(next_token(macro, position));
    }
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
