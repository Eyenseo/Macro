#ifndef cad_macro_IndentStream_h
#define cad_macro_IndentStream_h

#include "cad/macro/IndentBuffer.h"

#include <iostream>

namespace cad {
namespace macro {
class IndentStream : public std::ostream {
  IndentBuffer buf;

public:
  IndentStream(std::ostream& os);

  IndentStream& indent();
  IndentStream& dedent();
};
}
}
#endif
