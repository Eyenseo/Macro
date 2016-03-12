#ifndef cad_macro_IndentBuffer_h
#define cad_macro_IndentBuffer_h

#include <streambuf>

namespace cad {
namespace macro {
class IndentBuffer : public std::streambuf {
  long indention_;
  long indention_step_;
  std::streambuf* sbuf;
  std::string buffer_;
  bool indented_;

  void print_out();
  int_type overflow(int_type c) override;

public:
  IndentBuffer(std::streambuf* const s, const long indention_step,
               const long indention);
  IndentBuffer(std::streambuf* const s, const long indention_step);
  IndentBuffer(std::streambuf* const s);
  ~IndentBuffer();

  void indent();
  void dedent();
};
}
}
#endif
