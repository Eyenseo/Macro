#include "cad/macro/IndentBuffer.h"

namespace cad {
namespace macro {
IndentBuffer::IndentBuffer(std::streambuf* const s, const long indention_step,
                           const long indention)
    : indention_(indention)
    , indention_step_(indention_step)
    , sbuf(s)
    , indented_(false) {
}

IndentBuffer::IndentBuffer(std::streambuf* const s, const long indention_step)
    : IndentBuffer(s, indention_step, 0) {
}

IndentBuffer::IndentBuffer(std::streambuf* const s)
    : IndentBuffer(s, 2) {
}

IndentBuffer::~IndentBuffer() {
  print_out();
}

void IndentBuffer::print_out() {
  if(!buffer_.empty()) {
    sbuf->sputn(buffer_.c_str(), buffer_.size());
    buffer_.clear();
  }
}

IndentBuffer::int_type IndentBuffer::overflow(int_type c) {
  if(traits_type::eq_int_type(traits_type::eof(), c)) {
    return traits_type::not_eof(c);
  }

  if(!indented_ && c != '\n' && c != '\r') {
    indented_ = true;
    buffer_ = std::string(static_cast<unsigned long>(indention_), ' ');
  }

  buffer_ += static_cast<char>(c);

  if(c == '\n' || c == '\r') {
    print_out();
    indented_ = false;
  }

  return c;
}

void IndentBuffer::indent() {
  indention_ += indention_step_;
}
void IndentBuffer::dedent() {
  indention_ -= indention_step_;
  if(indention_ < 0) {
    indention_ = 0;
  }
}
}
}
