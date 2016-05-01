#ifndef cad_macro_IndentBuffer_h
#define cad_macro_IndentBuffer_h

#include <streambuf>

namespace cad {
namespace macro {
/**
 * @brief  The class is used by IndentStream to add indention to std::ostreams
 */
class IndentBuffer : public std::streambuf {
  long indention_;
  long indention_step_;
  std::streambuf* sbuf;
  std::string buffer_;
  bool indented_;

  void print_out();
  int_type overflow(int_type c) override;

public:
  /**
   * @brief  Ctor
   *
   * @param  s               stream to indent the input for
   * @param  indention_step  The indention step
   * @param  indention       The initial indention
   */
  IndentBuffer(std::streambuf* const s, const long indention_step,
               const long indention);
  /**
   * @brief  Ctor
   *
   * @param  s               stream to indent the input for
   * @param  indention_step  The indention step
   */
  IndentBuffer(std::streambuf* const s, const long indention_step);
  /**
   * @brief  Ctor
   *
   * @param  s     stream to indent the input for
   */
  IndentBuffer(std::streambuf* const s);
  /**
   * @brief  Dtor
   */
  ~IndentBuffer();

  void indent();
  void dedent();
};
}
}
#endif
