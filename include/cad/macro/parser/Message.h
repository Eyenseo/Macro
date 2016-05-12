#ifndef cad_macro_parser_Message_h
#define cad_macro_parser_Message_h

#include "cad/macro/parser/Token.h"

#include <sstream>
#include <string>
#include <vector>

namespace cad {
namespace macro {
namespace parser {
/**
 * @brief  The Message class is a convenience class that provides nicely
 *         formated Exceptions
 */
class Message {
  template <typename STREAME, typename STREAMER>
  struct is_streamable_to {
    template <typename T, typename U>
    static auto test(void*)
        -> decltype(std::declval<U&>() << std::declval<T&>(), std::true_type());

    template <typename, typename>
    static std::false_type test(...);

    static const bool value =
        std::is_same<std::true_type,
                     decltype(test<STREAME, STREAMER>(nullptr))>::value;
  };

  std::reference_wrapper<const Token> token_;
  std::string file_;
  std::string message_;

public:
  /**
   * @brief  Ctor
   *
   * @param  token  The token the error is about
   * @param  file   The file name / macro name
   */
  Message(std::reference_wrapper<const Token> token, std::string file);

  /**
   * @brief  Stream operator to add information to the Message
   *
   * @param  base       The base (this)
   * @param  s          The instance to stream / add to the Message
   *
   * @return this
   */
  template <typename Ty, typename = typename std::enable_if<is_streamable_to<
                             Ty, std::stringstream>::value>::type>
  friend Message& operator<<(Message& base, Ty s) noexcept(true) {
    std::stringstream ss;
    ss << base.message_ << s;
    base.message_ = ss.str();

    return base;
  }

  /**
   * @return The message this instance represents
   */
  std::string message() const;
};
}
}
}
#endif
