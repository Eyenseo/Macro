#ifndef cad_macro_IndentStream_h
#define cad_macro_IndentStream_h

#include "cad/macro/IndentBuffer.h"

#include <iostream>

namespace cad {
namespace macro {
/**
 * @brief  The IndentStream is a wrapper class for any std::ostream instances
 *         and makes them indentable
 */
class IndentStream : public std::ostream {
  IndentBuffer buf;

public:
  /**
   * @brief  Ctor
   *
   * @param  os         Underling stream that will be wrapped
   * @param  step       The indention size
   * @param  indention  The initial indention
   */
  IndentStream(std::ostream& os, const long step = 4, const long indention = 0);

  /**
   * @brief  Indents the following input
   *
   * @return this IndentStream
   */
  IndentStream& indent();
  /**
   * @brief  Dedents the following input
   *
   * @return this IndentStream
   */
  IndentStream& dedent();
};
}
}
#endif
