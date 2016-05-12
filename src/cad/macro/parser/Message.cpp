#include "cad/macro/parser/Message.h"

namespace cad {
namespace macro {
namespace parser {
Message::Message(std::reference_wrapper<const Token> token, std::string file)
    : token_(std::move(token))
    , file_(std::move(file)) {
}

std::string Message::message() const {
  std::stringstream ss;

  ss << file_ << ':' << token_.get().line << ':' << token_.get().column << ": "
     << message_;
  if(token_.get().source_line) {
    ss << '\n'
       << *token_.get().source_line << '\n'
       << std::string(token_.get().column - 1, ' ') << "^\n";
  }

  return ss.str();
}
}
}
}
