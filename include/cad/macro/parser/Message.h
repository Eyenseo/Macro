#ifndef cad_macro_parser_Message_h
#define cad_macro_parser_Message_h

#include "cad/macro/parser/Token.h"

#include <vector>
#include <string>
#include <sstream>

namespace cad {
namespace macro {
namespace parser {
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
  Message(std::reference_wrapper<const Token> token, std::string file);

  template <typename Ty, typename = typename std::enable_if<is_streamable_to<
                             Ty, std::stringstream>::value>::type>
  friend Message& operator<<(Message& base, Ty s) noexcept(true) {
    std::stringstream ss;
    ss << base.message_ << s;
    base.message_ = ss.str();

    return base;
  }

  std::string message() const;
};
}
}
}
#endif
