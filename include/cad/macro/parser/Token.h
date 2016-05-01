#ifndef cad_macro_parser_Token_h
#define cad_macro_parser_Token_h

#include <memory>
#include <string>

namespace cad {
namespace macro {
namespace parser {
/**
 * @brief  The Token struct represents a collection of characters of the macro
 *         that are a token.
 */
struct Token {
  size_t line;
  size_t column;
  std::string token;
  std::shared_ptr<std::string> source_line;

  /**
   * @brief  Ctor
   */
  Token();
  /**
   * @brief  Ctor
   *
   * @param  line         The line
   * @param  column       The column
   * @param  token        The token
   * @param  source_line  The source line
   */
  Token(size_t line, size_t column, std::string token,
        std::shared_ptr<std::string> source_line = nullptr);

  /**
   * @brief  Equality comparison
   *
   * @param  other  The Token to compare against
   *
   * @return true if the two objects are same
   */
  bool operator==(const Token& other) const;
  /**
   * @brief  Equality comparison
   *
   * @param  other  The Token to compare against
   *
   * @return false if the two objects are same
   */
  bool operator!=(const Token& other) const;
};
/**
 * @brief  Stream operator that will pretty print the Token
 *
 * @param  os     The stream to print the Token into
 * @param  token  The Token to print
 *
 * @return the input stream
 */
std::ostream& operator<<(std::ostream& os, const Token& token);
}
}
}
#endif
